#ifndef VMM_H_
#define VMM_H_

#include <mm/pmm.h>

#define KERNEL_HIGH_VMA 0xffffffff80000000
#define HIGH_VMA 0xffff800000000000

#define GET_PMLX_FLAGS(pmlX) ((pmlX) & 0xfff)
#define GET_PMLX_ADDR(pmlX) ((pmlX) & ~(0xfff))

typedef struct {
    uint64_t *pml4;
    int lock;
} pagestruct_t;

void page_map(pagestruct_t *p, uint64_t paddr, uint64_t vaddr, uint64_t flags, uint64_t flags1);

void page_unmap(pagestruct_t *p, uint64_t vaddr, uint64_t flags);

void page_copy(pagestruct_t *in, pagestruct_t *out);

uint64_t grab_PML4();

void tlb_flush();

void pagestruct_init(pagestruct_t *in);

void vmm_init();

#endif
