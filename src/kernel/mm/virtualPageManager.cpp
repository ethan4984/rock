#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <kernel/mm/kHeap.h>
#include <lib/memoryUtils.h>
#include <lib/output.h>

namespace vmm {

mapping *mapping::mappings;
uint64_t mapping::mappingCnt = 0;

struct mappingIndexes {
    mappingIndexes(uint64_t addr) {
        pml4idx = (addr & ((uint64_t)0x1ff << 39)) >> 39;
        pml3idx = (addr & ((uint64_t)0x1ff << 30)) >> 30;
        pml2idx = (addr & ((uint64_t)0x1ff << 21)) >> 21;
        pml1idx = (addr & ((uint64_t)0x1ff << 12)) >> 12;

        pml4 = (uint64_t*)(grabPML4() + HIGH_VMA);
        pml3 = (uint64_t*)((pml4[pml4idx] & ~(0xfff)) + HIGH_VMA);

        if((uint64_t)pml3 != HIGH_VMA) 
            pml2 = (uint64_t*)((pml3[pml3idx] & ~(0xfff)) + HIGH_VMA);
        else {
            pml3 = NULL;
            pml2 = NULL;
        }

        if((uint64_t)pml2 != HIGH_VMA)
            pml1 = (uint64_t*)((pml2[pml2idx] & ~(0xfff)) + HIGH_VMA);
        else
            pml1 = NULL;
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

void mapping::copy(mapping m1) {
    pml4 = (uint64_t*)(pmm::alloc(1) + HIGH_VMA);
    memcpy64(pml4, m1.pml4, 0x200);

    for(int i = 0; i < 0x200; i++) {
        if(m1.pml4[i] & 0b1) {
            pml4[i] = pmm::alloc(1) | GET_PMLX_FLAGS(m1.pml4[i]);
            uint64_t *pml3 = (uint64_t*)(GET_PMLX_ADDR(pml4[i]) + HIGH_VMA), *m1pml3 = (uint64_t*)(GET_PMLX_ADDR(m1.pml4[i]) + HIGH_VMA);
            memcpy64(pml3, m1pml3, 0x200);

            for(int j = 0; j < 0x200; j++) {
                if(m1pml3[j] & 0b1 && !(m1pml3[j] & (1 << 7))) {
                    pml3[j] = pmm::alloc(1) | GET_PMLX_FLAGS(m1pml3[j]);
                    uint64_t *pml2 = (uint64_t*)(GET_PMLX_ADDR(pml3[j]) + HIGH_VMA), *m1pml2 = (uint64_t*)(GET_PMLX_ADDR(m1pml3[j])  + HIGH_VMA);
                    memcpy64(pml2, m1pml2, 0x200);

                    for(int k = 0; k < 0x200; k++) {
                        if(m1pml2[k] & 0b1 && !(m1pml2[k] & (1 << 7))) {
                            pml2[k] = pmm::alloc(1) | GET_PMLX_FLAGS(m1pml2[k]);
                            uint64_t *pml1 = (uint64_t*)(GET_PMLX_ADDR(pml2[k]) + HIGH_VMA), *m1pml1 = (uint64_t*)(GET_PMLX_ADDR(m1pml2[k]) + HIGH_VMA);
                            memcpy64(pml1, m1pml1, 0x200);
                        }
                    }
                }
            }
        }
    }
}

static void freeMapping(uint64_t *pmlX) {
    static size_t cnt = 4; 

    for(int i = 0; i < 0x200; i++) { 
        if(pmlX[i] & 0b1 && !(pmlX[i] & (1 << 7))) {
            if(cnt > 0 && cnt <= 4) {
                freeMapping((uint64_t*)(GET_PMLX_ADDR(pmlX[i]) + HIGH_VMA));
                cnt--;
            }
            pmm::free(pmlX[i], 1);
        }
    }

    cnt = 4;
}

void mapping::exit() {
    if(pml4 != NULL)
        freeMapping(pml4);
}

void mapping::init() {
    asm volatile ("movq %0, %%cr3" :: "r" ((uint64_t)pml4 - HIGH_VMA) : "memory");
}

void mapping::addMapping(mapping newMapping) {
    if(mappingCnt + 1 % 10 == 0) {
        mappings = (mapping*)kheap.krealloc(mappings, 10);
    }
    
    mappings[mappingCnt++] = newMapping;
}

void mapping::mappingsInit() {
    mappingCnt = 0;
    mappings = new mapping[10];
}

void map(uint64_t physicalAddr, uint64_t virtualAddr, uint64_t flags, uint64_t flags1) {
    mappingIndexes indexes(virtualAddr);

    if(indexes.pml3 == NULL) {
        indexes.pml3 = (uint64_t*)(pmm::alloc(1) + HIGH_VMA);
        indexes.pml4[indexes.pml4idx] = ((uint64_t)indexes.pml3 - HIGH_VMA) | flags; 
    }

    if(indexes.pml2 == NULL) {
        indexes.pml2 = (uint64_t*)(pmm::alloc(1) + HIGH_VMA);
        indexes.pml3[indexes.pml3idx] = ((uint64_t)indexes.pml2 - HIGH_VMA) | flags; 
    }

    if(!(flags1 & (1 << 7))) { // check for 4kb pages
        if(indexes.pml1 == NULL) {
            indexes.pml1 = (uint64_t*)(pmm::alloc(1) + HIGH_VMA);
            indexes.pml2[indexes.pml2idx] = ((uint64_t)indexes.pml1 - HIGH_VMA) | flags;
        }

        indexes.pml1[indexes.pml1idx] = physicalAddr | flags1;
    } else {
        indexes.pml2[indexes.pml2idx] = physicalAddr | flags1;
    }

    tlbFlush();
}

void unmap(uint64_t virtualAddr, uint64_t flags) {
    mappingIndexes indexes(virtualAddr);

    if(flags & (1 << 7)) { // check for 2mb pages
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

    mapping::mappingsInit();
    mapping::addMapping(mapping { pml4 } );

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
