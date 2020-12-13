#include <mm/vmm.h>
#include <bitmap.h>
#include <output.h>

typedef struct {
    uint64_t pml4idx;
    uint64_t pml3idx;
    uint64_t pml2idx;
    uint64_t pml1idx;

    uint64_t *pml3;
    uint64_t *pml2;
    uint64_t *pml1;
} page_index_t;

static pagestruct_t kernel_mapping;

static void get_page_indexes(uint64_t addr, uint64_t *pml4, page_index_t *ret) {
    ret->pml4idx = (addr & ((uint64_t)0x1ff << 39)) >> 39;
    ret->pml3idx = (addr & ((uint64_t)0x1ff << 30)) >> 30;
    ret->pml2idx = (addr & ((uint64_t)0x1ff << 21)) >> 21;
    ret->pml1idx = (addr & ((uint64_t)0x1ff << 12)) >> 12;

    ret->pml3 = (uint64_t*)((pml4[ret->pml4idx] & ~(0xfff)) + HIGH_VMA);

    if((uint64_t)ret->pml3 != HIGH_VMA) {
        ret->pml2 = (uint64_t*)((ret->pml3[ret->pml3idx] & ~(0xfff)) + HIGH_VMA);
    } else {
        ret->pml3 = NULL;
        ret->pml2 = NULL;
        ret->pml1 = NULL;
        return;
    }

    if((uint64_t)ret->pml2 != HIGH_VMA)
        ret->pml1 = (uint64_t*)((ret->pml2[ret->pml2idx] & ~(0xfff)) + HIGH_VMA);
    else
        ret->pml1 = NULL;
}

static void page_map(pagestruct_t *p, uint64_t paddr, uint64_t vaddr, uint64_t flags, uint64_t flags1) {
    page_index_t indexes;
    get_page_indexes(vaddr, p->pml4, &indexes);

    if(indexes.pml3 == NULL) {
        indexes.pml3 = (uint64_t*)(pmm_calloc(1) + HIGH_VMA);
        p->pml4[indexes.pml4idx] = ((uint64_t)indexes.pml3 - HIGH_VMA) | flags; 
    }

    if(indexes.pml2 == NULL) {
        indexes.pml2 = (uint64_t*)(pmm_calloc(1) + HIGH_VMA);
        indexes.pml3[indexes.pml3idx] = ((uint64_t)indexes.pml2 - HIGH_VMA) | flags; 
    }

    if(!(flags1 & (1 << 7))) { // check for 4kb pages
        if(indexes.pml1 == NULL) {
            indexes.pml1 = (uint64_t*)(pmm_calloc(1) + HIGH_VMA);
            indexes.pml2[indexes.pml2idx] = ((uint64_t)indexes.pml1 - HIGH_VMA) | flags;
        }
        
        indexes.pml1[indexes.pml1idx] = paddr | flags1;
    } else {
        indexes.pml2[indexes.pml2idx] = paddr | flags1;
    }

    tlb_flush();
}

static uint64_t page_unmap(pagestruct_t *p, uint64_t vaddr, uint64_t flags) { 
    page_index_t indexes;
    get_page_indexes(vaddr, p->pml4, &indexes);

    if(flags & (1 << 7)) { // check for 2mb pages
        uint64_t save = indexes.pml2[indexes.pml2idx];
        indexes.pml2[indexes.pml2idx] = 0;
        return save;
    } 

    uint64_t save = indexes.pml1[indexes.pml1idx];
    indexes.pml1[indexes.pml1idx] = 0;
    return save;
}

void map_range(pagestruct_t *p, uint64_t vaddr, uint64_t cnt, uint64_t flags) {
    for(uint64_t i = 0; i < cnt; i++) {
        page_map(p, pmm_calloc(1), vaddr, flags, flags);
    }

    tlb_flush();
} 

void unmap_range(pagestruct_t *p, uint64_t vaddr, uint64_t cnt, uint64_t flags) {
    for(uint64_t i = 0; i < cnt; i++) {
        pmm_free(page_unmap(p, vaddr, flags), 1);     
    }

    tlb_flush();
} 

void page_copy(pagestruct_t *in, pagestruct_t *out) {
    memcpy64(in->pml4, out->pml4, 0x200);

    for(int i = 0; i < 0x200; i++) {
        if(out->pml4[i] & 0b1) {
            in->pml4[i] = pmm_calloc(1) | GET_PMLX_FLAGS(out->pml4[i]);
            uint64_t *pml3 = (uint64_t*)(GET_PMLX_ADDR(in->pml4[i]) + HIGH_VMA), *m1pml3 = (uint64_t*)(GET_PMLX_ADDR(out->pml4[i]) + HIGH_VMA);
            memcpy64(pml3, m1pml3, 0x200);

            for(int j = 0; j < 0x200; j++) {
                if(m1pml3[j] & 0b1 && !(m1pml3[j] & (1 << 7))) {
                    pml3[j] = pmm_calloc(1) | GET_PMLX_FLAGS(m1pml3[j]);
                    uint64_t *pml2 = (uint64_t*)(GET_PMLX_ADDR(pml3[j]) + HIGH_VMA), *m1pml2 = (uint64_t*)(GET_PMLX_ADDR(m1pml3[j])  + HIGH_VMA);
                    memcpy64(pml2, m1pml2, 0x200);

                    for(int k = 0; k < 0x200; k++) {
                        if(m1pml2[k] & 0b1 && !(m1pml2[k] & (1 << 7))) {
                            pml2[k] = pmm_calloc(1) | GET_PMLX_FLAGS(m1pml2[k]);
                            uint64_t *pml1 = (uint64_t*)(GET_PMLX_ADDR(pml2[k]) + HIGH_VMA), *m1pml1 = (uint64_t*)(GET_PMLX_ADDR(m1pml2[k]) + HIGH_VMA);
                            memcpy64(pml1, m1pml1, 0x200);
                        }
                    }
                }
            }
        }
    }
}

uint64_t grab_PML4() {
    uint64_t pml4;
    asm volatile ("movq %%cr3, %0" : "=r"(pml4));
    return pml4;
}

void tlb_flush() {
    asm volatile ("movq %0, %%cr3" :: "r" (grab_PML4()) : "memory");
}

void pagestruct_init(pagestruct_t *in) {
    asm volatile ("movq %0, %%cr3" :: "r" ((uint64_t)in->pml4 - HIGH_VMA) : "memory");
}

void vmm_init() {
    kernel_mapping = (pagestruct_t) {   .pml4 = (uint64_t*)(pmm_calloc(1) + HIGH_VMA),
                                        .bitmap = kcalloc(0x1000),
                                        .bm_size = 0x1000
                                    };

    page_copy(&kernel_mapping, &(pagestruct_t) { .pml4 = (uint64_t*)(grab_PML4() + HIGH_VMA) });
    pagestruct_init(&kernel_mapping);
}
