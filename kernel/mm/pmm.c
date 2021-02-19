#include <mm/pmm.h>
#include <mm/vmm.h>
#include <stivale.h>
#include <debug.h>

static uint8_t *bitmap;
size_t total_physical_mem = 0;

void pmm_init(struct stivale *stivale) {
    struct stivale_mmap *mmap = (struct stivale_mmap*)stivale->mmap_addr;
    for(size_t i = 0; i < stivale->mmap_cnt; i++) {
        total_physical_mem += mmap[i].len;
        kprintf("[KMM] [%x -> %x] : length %x type %x\n", mmap[i].addr, mmap[i].addr + mmap[i].len, mmap[i].len, mmap[i].type);
    }
    
    size_t bm_size = total_physical_mem / PAGE_SIZE / sizeof(uint8_t);

    for(size_t i = 0; i < stivale->mmap_cnt; i++) {
        if(mmap[i].type == 1 && mmap[i].len >= bm_size) {
            bitmap = (uint8_t*)(mmap[i].addr + HIGH_VMA);
            memset64((uint64_t*)bitmap, ~(size_t)0, bm_size / sizeof(uint8_t));
            break;
        }
    }

    for(size_t i = 0; i < stivale->mmap_cnt; i++) {
        if(mmap[i].type == 1) {
            bm_free_region(bitmap, mmap[i].addr / PAGE_SIZE, mmap[i].len / PAGE_SIZE);
        }
    }
    
    bm_alloc_region(bitmap, ((size_t)bitmap - HIGH_VMA) / PAGE_SIZE, bm_size / PAGE_SIZE);
    bm_alloc_region(bitmap, 0, 0x100000 / PAGE_SIZE); 

    kprintf("[KMM] Total detected memory: %x\n", total_physical_mem);
}

uint64_t pmm_alloc(uint64_t cnt) {
    size_t base = bm_first_free(bitmap, total_physical_mem / PAGE_SIZE) * PAGE_SIZE, count = 0;
    for(size_t i = bm_first_free(bitmap, total_physical_mem / PAGE_SIZE); i < total_physical_mem / PAGE_SIZE; i++) {
        if(BM_TEST(bitmap, i)) {
            base += (count + 1) * PAGE_SIZE;
            count = 0;
            continue;
        }

        if(++count == cnt) {
            for(size_t j = 0; j < count; j++)
                BM_SET(bitmap, base / PAGE_SIZE + j);
            return base;
        }
    }
    
    return 0;
}

uint64_t pmm_calloc(uint64_t cnt) {
    uint64_t ret = pmm_alloc(cnt);
    memset64((uint64_t*)(ret + HIGH_VMA), 0, (cnt * PAGE_SIZE) / 8);
    return ret;
}

void pmm_free(uint64_t base, uint64_t cnt) {
    for(size_t i = DIV_ROUNDUP((size_t)base, PAGE_SIZE); i < DIV_ROUNDUP((size_t)base, PAGE_SIZE) + cnt; i++) {
        BM_CLEAR(bitmap, i);
    }
}
