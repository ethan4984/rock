#ifndef VMM_H_
#define VMM_H_

#define KERNEL_HIGH_VMA 0xffffffff80000000
#define HIGH_VMA 0xffff800000000000
#define PAGE_SIZE 0x1000

#include <stdint.h>
#include <stddef.h>

struct page_map {
    uint64_t *pml4;
    uint8_t *bitmap;
    size_t bm_size;
    size_t lock;
};

extern struct page_map kernel_mapping; 

void vmm_map_range(struct page_map *page_map, size_t vaddr, size_t cnt, uint64_t flags, uint64_t flags1);
void vmm_unmap_range(struct page_map *page_map, size_t vaddr, size_t cnt, uint64_t flags1); 
void vmm_map_page(struct page_map *page_map, size_t paddr, size_t vaddr, uint64_t flags, uint64_t flags1);
size_t vmm_unmap_page(struct page_map *page_map, size_t vaddr, uint64_t flags1);
void vmm_tlb_flush();
void vmm_init();
void vmm_page_map_init(struct page_map *page_map);
uint64_t vmm_get_pml4(); 
struct page_map *vmm_generic_page_map();

#endif
