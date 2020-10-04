#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <kernel/mm/kHeap.h>
#include <lib/memoryUtils.h>
#include <lib/output.h>

namespace vmm {

struct mappingIndexes {
    mappingIndexes(uint64_t addr) {
        pml4idx = (addr & ((uint64_t)0x1ff << 39)) >> 39;
        pml3idx = (addr & ((uint64_t)0x1ff << 30)) >> 30;
        pml2idx = (addr & ((uint64_t)0x1ff << 21)) >> 21;
        pml1idx = (addr & ((uint64_t)0x1ff << 12)) >> 12;

        pml4 = (uint64_t*)(grabPML4() + HIGH_VMA);
        pml3 = (uint64_t*)((pml4[pml4idx] & ~(0xfff)) + HIGH_VMA);
        pml2 = (uint64_t*)((pml3[pml3idx] & ~(0xfff)) + HIGH_VMA);
        pml1 = (uint64_t*)((pml2[pml2idx] & ~(0xfff)) + HIGH_VMA);
    }

    uint64_t pml4idx;
    uint64_t pml3idx;
    uint64_t pml2idx;
    uint64_t pml1idx;

    uint64_t *pml4;
    uint64_t *pml3;
    uint64_t *pml2;
    uint64_t *pml1;
};

void unmap(uint64_t virtualAddr, uint64_t flags) {
    mappingIndexes indexes(virtualAddr);

    if(flags & (1 << 7)) {
        indexes.pml2[indexes.pml2idx] = 0;
    } else {
        indexes.pml1[indexes.pml1idx] = 0;
    }

    tlbFlush();
}

void init() {
    uint64_t *pml4 = (uint64_t*)(pmm::alloc(1) + HIGH_VMA);
    uint64_t *pml3 = (uint64_t*)(pmm::alloc(1) + HIGH_VMA);

    uint64_t *pml3_HH = (uint64_t*)(pmm::alloc(1) + HIGH_VMA);
    uint64_t *pml2_HH = (uint64_t*)(pmm::alloc(1) + HIGH_VMA);

    uint64_t *pml2_1G = (uint64_t*)(pmm::alloc(1) + HIGH_VMA);
    uint64_t *pml2_2G = (uint64_t*)(pmm::alloc(1) + HIGH_VMA);
    uint64_t *pml2_3G = (uint64_t*)(pmm::alloc(1) + HIGH_VMA);
    uint64_t *pml2_4G = (uint64_t*)(pmm::alloc(1) + HIGH_VMA);

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
}


void tlbFlush() {
    asm volatile ("movq %0, %%cr3" :: "r" (grabPML4()) : "memory");
}

uint64_t grabPML4() {
    uint64_t pml4;
    asm volatile ("movq %%cr3, %0" : "=r"(pml4));
    return pml4;
}

}
