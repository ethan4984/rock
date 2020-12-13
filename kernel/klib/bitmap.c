#include <bitmap.h>
#include <output.h>

typedef struct {
    uint32_t blk_cnt;
    uint32_t blk_idx;
} alloc_t;

static uint8_t *bitmap;
static alloc_t *allocs;
static uint64_t start, bitmap_size, allocation_size;

static int64_t first_free();
static int64_t first_free_alloc();

void bitmap_init() {
    start = pmm_alloc(0x1000);

    bitmap = (uint8_t*)(pmm_alloc(ROUNDUP(0x1000 * 0x1000, BLOCK_SIZE) / 8) + HIGH_VMA);
    bitmap_size = ROUNDUP(0x1000 * 0x1000, BLOCK_SIZE) / 8;
	
    allocs = (alloc_t*)(pmm_alloc(0x20) + HIGH_VMA);
    allocation_size = (0x20 * 0x1000);
	
    memset8(bitmap, 0, bitmap_size);
    memset8((uint8_t*)allocs, 0, allocation_size);
}

void *kmalloc(uint64_t size) {
    uint32_t cnt = 0, block_count = ROUNDUP(size, BLOCK_SIZE);
    void *base = (void*)(first_free() * BLOCK_SIZE); 

    for(uint64_t i = first_free(); i < bitmap_size; i++) {
        if(BM_TEST(bitmap, i)) {
            base = (void*)((uint64_t)base + (cnt + 1) * BLOCK_SIZE);
            cnt = 0;
            continue;
        }

        if(++cnt == block_count) {
            uint64_t slot = first_free_alloc();

            allocs[slot] = (alloc_t) { block_count, (uint32_t)((uint64_t)base / BLOCK_SIZE) };

            for(int64_t j = 0; j < block_count; j++) {
                BM_SET(bitmap, (uint64_t)base / BLOCK_SIZE + j);
            }

            return (void*)((uint64_t)base + start + HIGH_VMA);
        }
    }

    kprintf("[KDEBUG]", "heap is full");
    return NULL;
}

uint64_t kfree(void *addr) {
    uint64_t bitmap_base = ((uint64_t)addr - HIGH_VMA - start) / BLOCK_SIZE, size = 0;

    size_t i;
    for(i = 0; i < allocation_size; i++) {
        if(allocs[i].blk_idx == bitmap_base)
            break;
    }

    size = allocs[i].blk_cnt;

    for(uint64_t j = bitmap_base; j < bitmap_base + size; j++) {
        BM_CLEAR(bitmap, j);
    }

    allocs[i] = (alloc_t) { 0 };

    return size;
}

void *krealloc(void *addr, uint64_t size) {
    uint64_t alloc_size = kfree(addr);
    void *new_addr = kmalloc(size);
    memcpy64((uint64_t*)new_addr, (uint64_t*)addr, (alloc_size * BLOCK_SIZE) / 8);
    return new_addr;
}

void *krecalloc(void *addr, uint64_t size) {
    uint64_t alloc_size = kfree(addr);
    void *new_addr = kcalloc(size);
    memcpy64((uint64_t*)new_addr, (uint64_t*)addr, (alloc_size * BLOCK_SIZE) / 8);
    return new_addr;
}

void *kcalloc(uint64_t cnt) {
    uint8_t *ret = kmalloc(cnt);
    memset8(ret, 0, cnt);
    return ret;
}

static int64_t first_free() {
    for(uint64_t i = 0; i < bitmap_size; i++) {
        if(!BM_TEST(bitmap, i))
            return i;
    }
    return -1;    
}

static int64_t first_free_alloc() {
	for(size_t i = 0; i < allocation_size; i++) {
        if(allocs[i].blk_cnt == 0) {
            return i;
        }
    }
    return -1;
}
