#ifndef VMM_H_
#define VMM_H_

#include <mm/pmm.h>

#define KERNEL_HIGH_VMA 0xffffffff80000000
#define HIGH_VMA 0xffff800000000000
#define PAGE_SIZE 0x1000

#define GET_PMLX_FLAGS(pmlX) ((pmlX) & 0xfff)
#define GET_PMLX_ADDR(pmlX) ((pmlX) & ~(0xfff))

typedef struct {
    uint64_t *pml4;
    int lock;
} pagestruct_t;

extern pagestruct_t kernel_mapping;

void map_range(pagestruct_t *p, uint64_t vaddr, uint64_t cnt, uint64_t flags);
void unmap_range(pagestruct_t *p, uint64_t vaddr, uint64_t cnt, uint64_t flags);
pagestruct_t *vmm_generic_pagestruct();
void page_copy(pagestruct_t *in, pagestruct_t *out);
uint64_t grab_PML4();
void tlb_flush();
void pagestruct_init(pagestruct_t *in);
void vmm_init();

#endif
