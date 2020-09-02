#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <kernel/mm/kHeap.h>
#include <lib/memoryUtils.h>
#include <lib/output.h>

namespace kernel {

uint64_t kpml3;
uint64_t kpml3HH;

pdEntry_t *virtualPageManager_t::pdEntries = NULL;

pdEntry_t::pdEntry_t(uint64_t pml4, uint64_t pdFlags, uint64_t ptFlags, uint64_t hashIdentifier) : 
    pml4(pml4), pdFlags(pdFlags), ptFlags(ptFlags) 
{

}

void virtualPageManager_t::unmap(uint64_t base, uint64_t cnt, uint64_t flags) {
    uint64_t *pml4, *pml3, *pml2, *pml1 = NULL;

    pageMap_t pm = getIndexes(base, flags);

    pml4 = (uint64_t*)(grabPML4() + HIGH_VMA);
    pml3 = (uint64_t*)(pml4[pm.pml4Index] + HIGH_VMA); 
    pml2 = (uint64_t*)(pml3[pm.pml3Index] + HIGH_VMA); 

    kprintDS("[KDEBUG]", "%d %d %d %d", pm.pml4Index, pm.pml3Index, pm.pml2Index, pm.pml1Index);

    if(pm.pml1Index != -1) {
        pml1 = (uint64_t*)(pml2[pm.pml2Index] + HIGH_VMA);       
        memset64(pml1 + pm.pml1Index, 0, cnt);
        return;
    }

    memset64((uint64_t*)((uint64_t)pml2 + (pm.pml2Index * 8)), 0, cnt);

    tlbFlush();
}

void virtualPageManager_t::map(uint64_t base, uint64_t physicalBase, uint64_t cnt, uint64_t flags) {
    uint64_t *pml4, *pml3, *pml2, *pml1 = NULL, *pt;

    uint64_t pageSize = (flags & (1 << 7)) ? 0x200000 : 0x1000;

    pageMap_t pm = getIndexes(base, flags);

    pml4 = (uint64_t*)(grabPML4() + HIGH_VMA);
    pml3 = (uint64_t*)(pml4[pm.pml4Index] + HIGH_VMA); 
    pml2 = (uint64_t*)(pml3[pm.pml3Index] + HIGH_VMA); 

    if(pageSize == 0x1000) {
        pt = pml1;
    } else {
        pt = pml2;
    } 

    for(uint64_t i = pm.pml1Index; i < pm.pml1Index + cnt; i++) {
        pt[i] = physicalBase | flags; 
        physicalBase += pageSize;  
    }

    tlbFlush(); 
} 

pageMap_t virtualPageManager_t::getIndexes(uint64_t base, uint64_t flags) {
    int64_t pml4Index = -1, pml3Index = -1, pml2Index = -1, pml1Index = -1;

    pml4Index = base / (0x1000000000000 * 256);
    base %= (0x1000000000000 * 256);
    pml3Index = base / 0x8000000000;
    base %= 0x8000000000;
    pml2Index = base / 0x40000000;
    base %= 0x40000000;

    if((flags & (1 << 7)) == 0) { 
        pml1Index = base / 0x1000;
    } else {
        pml2Index = base / 0x200000;
    }
    
    return pageMap_t { pml4Index, pml3Index, pml2Index, pml1Index };
}

uint64_t virtualPageManager_t::newUserMap(uint64_t pageCnt) {
    uint64_t *pml4 = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml3 = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml2 = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml1 = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);

    memset(pml4, 0, 0x1000);
    memset(pml3, 0, 0x1000);
    memset(pml2, 0, 0x1000);
    memset(pml1, 0, 0x1000);

    pml4[256] = kpml3 | KERNEL_PD_FLAGS;
    pml4[511] = kpml3HH | KERNEL_PD_FLAGS;

    pml4[0] = ((uint64_t)&pml3 - HIGH_VMA) | USR_PD_FLAGS;
    pml3[0] = ((uint64_t)&pml2 - HIGH_VMA) | USR_PD_FLAGS;
    pml2[0] = ((uint64_t)&pml1 - HIGH_VMA) | USR_PD_FLAGS;  

    uint64_t physical = 0;
    for(uint64_t i = 0; i < pageCnt; i++) {
        pml1[i] = physical | USR_PT_FLAGS;
        physical += 0x1000;
    }
 
    pdEntry_t newEntry((uint64_t)pml4 - HIGH_VMA, USR_PT_FLAGS, USR_PD_FLAGS, 1);

    int64_t index = firstFreeSlot();
    if(index == -1) {
        entryCnt += 10;
        pdEntries = (pdEntry_t*)kheap.krealloc(pdEntries, sizeof(pdEntry_t) * 0x1000);
    }

    pdEntries[index] = newEntry;
    return index;
}

void pdEntry_t::initPageMap() {
    asm volatile("movq %0, %%cr3" :: "r" (pml4) : "memory");
} 

uint64_t virtualPageManager_t::grabPML4() {
    uint64_t pml4;
    asm volatile ("movq %%cr3, %0" : "=r"(pml4));
    return pml4;
}

uint64_t virtualPageManager_t::firstFreeSlot() {
    for(uint64_t i = 0; i < entryCnt; i++) {
        if(pdEntries[i].pml4 == 0) {
            return i; 
        }
    }
    return -1;
}

void virtualPageManager_t::initAddressSpace(uint64_t index) {
    pdEntries[index].initPageMap();
}

void virtualPageManager_t::init() {
    pdEntries = new pdEntry_t[0x1000];

    uint64_t *pml4 = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml3 = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);

    uint64_t *pml3_HH = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml2_HH = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);

    uint64_t *pml2_1G = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml2_2G = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml2_3G = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml2_4G = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);

    memset(pml4, 0, 0x8000);

    pml4[256] = ((uint64_t)&pml3[0] - HIGH_VMA) | KERNEL_PD_FLAGS;
    pml4[0] = ((uint64_t)&pml3[0] - HIGH_VMA) | KERNEL_PD_FLAGS;

    pml4[511] = ((uint64_t)&pml3_HH[0] - HIGH_VMA) | KERNEL_PD_FLAGS;
    pml3_HH[510] = ((uint64_t)&pml2_HH[0] - HIGH_VMA) | KERNEL_PD_FLAGS;

    pml3[0] = ((uint64_t)&pml2_1G[0] - HIGH_VMA) | KERNEL_PD_FLAGS;
    pml3[1] = ((uint64_t)&pml2_2G[0] - HIGH_VMA) | KERNEL_PD_FLAGS;
    pml3[2] = ((uint64_t)&pml2_3G[0] - HIGH_VMA) | KERNEL_PD_FLAGS;
    pml3[3] = ((uint64_t)&pml2_4G[0] - HIGH_VMA) | KERNEL_PD_FLAGS;

    uint64_t physical = 0;
    for(int i = 0; i < 512; i++) {
        pml2_1G[i] = physical | KERNEL_PT_FLAGS;
        pml2_2G[i] = (physical + 0x40000000) | KERNEL_PT_FLAGS;
        pml2_3G[i] = (physical + 0x80000000ull) | KERNEL_PT_FLAGS;
        pml2_4G[i] = (physical + 0xc0000000ull) | KERNEL_PT_FLAGS;

        pml2_HH[i] = physical | KERNEL_PT_FLAGS;

        physical += 0x200000;
    }

    asm volatile ("movq %0, %%cr3" :: "r" ((uint64_t)pml4 - HIGH_VMA) : "memory");

    pdEntry_t kernelEntry((uint64_t)pml4 - HIGH_VMA, KERNEL_PD_FLAGS, KERNEL_PT_FLAGS, 1);
    pdEntries[0] = kernelEntry;

    kpml3 = (uint64_t)pml3 - HIGH_VMA;
    kpml3HH = (uint64_t)pml3_HH - HIGH_VMA;
}


void virtualPageManager_t::tlbFlush() {
    asm volatile ("movq %0, %%cr3" :: "r" (grabPML4()) : "memory");
}

}
