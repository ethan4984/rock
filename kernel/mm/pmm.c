#include <mm/pmm.h>
#include <mm/vmm.h>
#include <output.h>

static uint8_t *bitmap = NULL;
size_t total_physical_mem = 0;

static void bm_alloc_region(uint64_t start, uint64_t length);
static void bm_free_region(uint64_t start, uint64_t length);
static int bm_first_free();

void pmm_init(stivale_t *stivale) {
    stivale_mmap_t *mmap = (stivale_mmap_t*)stivale->mmap_addr;
    for(uint64_t i = 0; i < stivale->mmap_cnt; i++) {
        total_physical_mem += mmap[i].len;
        kprintf("[KMM]", "[%x -> %x] : length %x type %x", mmap[i].addr, mmap[i].addr + mmap[i].len, mmap[i].len, mmap[i].type);
    }
    
    uint64_t bm_size = total_physical_mem / 0x1000 / 8;

    for(uint64_t i = 0; i < stivale->mmap_cnt; i++) {
        if((mmap[i].type == 1) && mmap[i].len >= bm_size) {
            bitmap = (uint8_t*)(mmap[i].addr + HIGH_VMA);
            memset64((uint64_t*)bitmap, ~(uint64_t)0, bm_size / 8);
            break;
        }
    }

    for(uint64_t i = 0; i < stivale->mmap_cnt; i++) {
        if(mmap[i].type == 1) {
            bm_free_region(mmap[i].addr, mmap[i].len);
        }
    }
    
    bm_alloc_region((uint64_t)bitmap - HIGH_VMA, bm_size);
    bm_alloc_region(0, 0x100000); // mark the frist 1mb reserved 

    kprintf("[KMM]", "Total detected memory: %x", total_physical_mem);
}

uint64_t pmm_alloc(uint64_t cnt) {
    uint64_t base = bm_first_free() * 0x1000, count = 0;
    for(uint64_t i = bm_first_free(); i < total_physical_mem / 0x1000; i++) {
        if(BM_TEST(bitmap, i)) {
            base += (count + 1) * 0x1000;
            count = 0;
            continue;
        }

        if(++count == cnt) {
            for(uint64_t j = 0; j < count; j++)
                BM_SET(bitmap, base / 0x1000 + j);
            return base;
        }
    }
    
    return 0;
}

uint64_t pmm_calloc(uint64_t cnt) {
    uint64_t ret = pmm_alloc(cnt);
    memset64((uint64_t*)(ret + HIGH_VMA), 0, (cnt * 0x1000) / 8);
    return ret;
}

void pmm_free(uint64_t base, uint64_t cnt) {
    for(uint64_t i = ROUNDUP((uint64_t)base, 0x1000); i < ROUNDUP((uint64_t)base, 0x1000) + cnt; i++) {
        BM_CLEAR(bitmap, i);
    }
}

static void bm_alloc_region(uint64_t start, uint64_t limit) {
    for(uint64_t i = start / 0x1000; i < (start / 0x1000) + ROUNDUP(limit, 0x1000); i++) {
        BM_SET(bitmap, i);
    }
}

static void bm_free_region(uint64_t start, uint64_t limit) { 
    for(uint64_t i = start / 0x1000; i < (start / 0x1000) + ROUNDUP(limit, 0x1000); i++) {
        BM_CLEAR(bitmap, i);
    }    
}

static int bm_first_free() { 
    for(uint64_t i = 0; i < total_physical_mem / 0x1000; i++) {
        if(!BM_TEST(bitmap, i)) {
            return i;
        }
    }
    return -1;
}
