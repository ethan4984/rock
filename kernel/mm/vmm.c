#include <mm/vmm.h>
#include <mm/pmm.h>
#include <mm/slab.h>

struct page_map_index {
    size_t pml4idx;
    size_t pml3idx; 
    size_t pml2idx;
    size_t pml1idx;

    uint64_t *pml3;
    uint64_t *pml2; 
    uint64_t *pml1;
};

struct page_map kernel_mapping;

static void get_page_index(struct page_map *page_map, size_t addr, struct page_map_index *ret) {
    *ret = (struct page_map_index) {    .pml4idx = (addr >> 39) & 0x1ff,
                                        .pml3idx = (addr >> 30) & 0x1ff,
                                        .pml2idx = (addr >> 21) & 0x1ff,
                                        .pml1idx = (addr >> 12) & 0x1ff
                                    };

    ret->pml3 = (uint64_t*)(page_map->pml4[ret->pml4idx] & ~(0xfff));
    if(ret->pml3 == NULL) {
        return;
    }

    ret->pml3 = (uint64_t*)((uint64_t)ret->pml3 + HIGH_VMA);
    ret->pml2 = (uint64_t*)(ret->pml3[ret->pml3idx] & ~(0xfff));
    if(ret->pml2 == NULL)
        return;

    ret->pml2 = (uint64_t*)((uint64_t)ret->pml2 + HIGH_VMA);
    ret->pml1 = (uint64_t*)(ret->pml2[ret->pml2idx] & ~(0xfff));
    if(ret->pml1 != NULL)
        ret->pml1 = (uint64_t*)((uint64_t)ret->pml1 + HIGH_VMA);
}

void vmm_map_page(struct page_map *page_map, size_t paddr, size_t vaddr, uint64_t flags, uint64_t flags1) { 
    struct page_map_index indices;
    get_page_index(page_map, vaddr, &indices);

    if(indices.pml3 == NULL) {
        indices.pml3 = (uint64_t*)(pmm_calloc(1) + HIGH_VMA); 
        page_map->pml4[indices.pml4idx] = ((uint64_t)indices.pml3 - HIGH_VMA) | flags;
    }

    if(indices.pml2 == NULL) {
        indices.pml2 = (uint64_t*)(pmm_calloc(1) + HIGH_VMA); 
        indices.pml3[indices.pml3idx] = ((uint64_t)indices.pml2 - HIGH_VMA) | flags;
    }

    if(!(flags1 & (1 << 7))) { // 4kb pages
        if(indices.pml1 == NULL) {
            indices.pml1 = (uint64_t*)(pmm_calloc(1) + HIGH_VMA);
            indices.pml2[indices.pml2idx] = ((uint64_t)indices.pml1 - HIGH_VMA) | flags;
        }
        
        indices.pml1[indices.pml1idx] = paddr | flags1; 
    } else { // 2mb pages
        indices.pml2[indices.pml2idx] = paddr | flags1;
    }
}

size_t vmm_unmap_page(struct page_map *page_map, size_t vaddr, uint64_t flags1) {
    struct page_map_index indices;
    get_page_index(page_map, vaddr, &indices);

    if(flags1 & (1 << 7)) { // 2mb pages
        uint64_t paddr = indices.pml2[indices.pml2idx];
        indices.pml2[indices.pml2idx] = 0;
        return paddr;
    }
    
    uint64_t paddr = indices.pml1[indices.pml1idx];
    indices.pml1[indices.pml1idx] = 0;
    return paddr;
}

uint64_t vmm_get_pml4() {
    uint64_t pml4;
    asm volatile ("movq %%cr3, %0" : "=r"(pml4));
    return pml4;
}

void vmm_tlb_flush() {
    asm volatile ("movq %0, %%cr3" :: "r" (vmm_get_pml4()) : "memory");
}

void vmm_map_range(struct page_map *page_map, size_t vaddr, size_t cnt, size_t flags, size_t flags1) {
    size_t page_size = PAGE_SIZE;

    if(flags & (1 << 7))
        page_size = 0x200000;

    for(size_t i = 0; i < cnt; i++) {
        vmm_map_page(page_map, pmm_calloc(1), vaddr, flags, flags1);
        vaddr += page_size;
    }

    vmm_tlb_flush();
}

void vmm_unmap_range(struct page_map *page_map, size_t vaddr, size_t cnt, size_t flags1) {
    size_t page_size = PAGE_SIZE;

    if(flags1 & (1 << 7)) 
        page_size = 0x200000;

    for(size_t i = 0; i < cnt; i++) {
        vmm_unmap_page(page_map, vaddr, flags1);
        vaddr += page_size;
    }

    vmm_tlb_flush(); 
}

void vmm_page_map_init(struct page_map *page_map) {
    asm volatile ("movq %0, %%cr3" :: "r" ((uint64_t)page_map->pml4 - HIGH_VMA) : "memory");
}

struct page_map *vmm_generic_page_map() { 
    struct page_map *page_map = kmalloc(sizeof(struct page_map));
    *page_map = (struct page_map) { .pml4 = (uint64_t*)(pmm_calloc(1) + HIGH_VMA) };

    page_map->pml4[256] = kernel_mapping.pml4[256];
    page_map->pml4[511] = kernel_mapping.pml4[511];

    return page_map;
}

void vmm_pagemap_copy(struct page_map *in, struct page_map *out) {
    in->pml4 = (void*)(pmm_calloc(1) + HIGH_VMA);
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

    in->bitmap = kmalloc(out->bm_size);
    memcpy8(in->bitmap, out->bitmap, out->bm_size);
    in->bm_size = out->bm_size;
}

void vmm_init() {
    kernel_mapping = (struct page_map) { .pml4 = (uint64_t*)(pmm_calloc(1) + HIGH_VMA) };

    size_t phys = 0;
    for(size_t i = 0; i < 0x200; i++) {
        vmm_map_page(&kernel_mapping, phys, KERNEL_HIGH_VMA + phys, 0x3, 0x3 | (1 << 7) | (1 << 8));
        phys += 0x200000;
    }

    phys = 0;
    for(size_t i = 0; i < total_physical_mem / 0x200000; i++) { 
        vmm_map_page(&kernel_mapping, phys, HIGH_VMA + phys, 0x3 | (1 << 2), 0x3 | (1 << 7) | (1 << 8) | (1 << 2));
        phys += 0x200000;
    }

    vmm_page_map_init(&kernel_mapping);
}
