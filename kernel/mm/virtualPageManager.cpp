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

uint64_t virtualPageManager_t::newUserMap(uint64_t pageCnt) {
    uint64_t *pml4 = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml3 = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml2 = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml1 = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);

    pml4[256] = kpml3 | USR_PD_FLAGS;
    pml4[511] = kpml3HH | USR_PD_FLAGS;

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

    uint64_t pdFlags = (1 << 2) | 0x3; /* set superuser/present/read/write bits */
    uint64_t ptFlags = (1 << 2) | (1 << 7) | 0x3; /* set superuser/size/present/read/write bits */

    uint64_t *pml4 = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml3 = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);

    uint64_t *pml3_HH = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml2_HH = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);

    uint64_t *pml2_1G = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml2_2G = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml2_3G = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);
    uint64_t *pml2_4G = (uint64_t*)(physicalPageManager.alloc(1) + HIGH_VMA);

    memset(pml4, 0, 0x8000);

    pml4[256] = ((uint64_t)&pml3[0] - HIGH_VMA) | pdFlags;
    pml4[0] = ((uint64_t)&pml3[0] - HIGH_VMA) | pdFlags;

    pml4[511] = ((uint64_t)&pml3_HH[0] - HIGH_VMA) | pdFlags;
    pml3_HH[510] = ((uint64_t)&pml2_HH[0] - HIGH_VMA) | pdFlags;

    pml3[0] = ((uint64_t)&pml2_1G[0] - HIGH_VMA) | pdFlags;
    pml3[1] = ((uint64_t)&pml2_2G[0] - HIGH_VMA) | pdFlags;
    pml3[2] = ((uint64_t)&pml2_3G[0] - HIGH_VMA) | pdFlags;
    pml3[3] = ((uint64_t)&pml2_4G[0] - HIGH_VMA) | pdFlags;

    uint64_t physical = 0;
    for(int i = 0; i < 512; i++) {
        pml2_1G[i] = physical | ptFlags | (1 << 7);
        pml2_2G[i] = (physical + 0x40000000) | ptFlags | (1 << 7);
        pml2_3G[i] = (physical + 0x80000000ull) | ptFlags | (1 << 7);
        pml2_4G[i] = (physical + 0xc0000000ull) | ptFlags | (1 << 7);

        pml2_HH[i] = physical | ptFlags | (1 << 7);

        physical += 0x200000;
    }

    asm volatile ("movq %0, %%cr3" :: "r" ((uint64_t)pml4 - HIGH_VMA) : "memory");

    pdEntry_t kernelEntry((uint64_t)pml4 - HIGH_VMA, pdFlags, pdFlags | (1 << 7), 1);
    pdEntries[0] = kernelEntry;

    kpml3 = (uint64_t)kpml3 - HIGH_VMA;
    kpml3HH = (uint64_t)kpml3HH - HIGH_VMA;
}

}
