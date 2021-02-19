#include <mm/vmm.h>
#include <sched/smp.h>
#include <fs/fd.h>
#include <debug.h>
#include <vec.h>

#define MAP_FAILED (void*)-1
#define MAP_PRIVATE 0x1
#define MAP_SHARED 0x2
#define MAP_FIXED 0x4
#define MAP_ANONYMOUS 0x8

#define MMAP_MIN_ADDR 0x10000

static void *mmap_alloc(struct page_map *page_map, uint64_t length) {
    uint64_t page_cnt = DIV_ROUNDUP(length, PAGE_SIZE), offset = 0;

    if(page_map->bitmap == NULL) {
        page_map->bm_size = 0x1000;
        page_map->bitmap = kcalloc(page_map->bm_size);
    }

    for(size_t i = 0, cnt = 0; i < page_map->bm_size; i++) {
        if(BM_TEST(page_map->bitmap, i)) {
            offset += cnt + 1;
            cnt = 0;
            continue;
        }

        if(++cnt == page_cnt) {
            void *ret = (void*)(offset * PAGE_SIZE + MMAP_MIN_ADDR);
            size_t index = ((size_t)ret - MMAP_MIN_ADDR) / PAGE_SIZE;

            for(uint64_t i = index; i < index + page_cnt; i++) {
                BM_SET(page_map->bitmap, i);
            }

            return ret;
        }
    }

    page_map->bm_size += 0x1000;
    page_map->bitmap = krecalloc(page_map->bitmap, page_map->bm_size);

    return mmap_alloc(page_map, length);
}

static int check_mmap_addr(struct page_map *page_map, int flags, void *addr, uint64_t length) {
    if(addr == NULL)
        return -1;

    if((uint64_t)addr % 0x1000 != 0) 
        return -1;

    if(flags == MAP_FIXED)
        return 0;

    size_t index = ((uint64_t)addr - MMAP_MIN_ADDR) / PAGE_SIZE;
    size_t page_cnt = DIV_ROUNDUP(length, PAGE_SIZE);

    for(uint64_t i = index; i < index + page_cnt; i++) { 
        if(BM_TEST(page_map->bitmap, i)) {
            return -1;
        }
    }
    
    return 0;
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, int64_t off) {
    struct core_local *local = get_core_local(CURRENT_CORE);

    size_t page_cnt = DIV_ROUNDUP(length, PAGE_SIZE);

    if(flags == MAP_ANONYMOUS) {
        if(check_mmap_addr(local->page_map, flags, addr, length) == -1) {
            addr = mmap_alloc(local->page_map, length);
        }

        vmm_map_range(local->page_map, (uint64_t)addr, page_cnt, 0x3 | (1 << 2), 0x3 | (1 << 2)); 
    } 

    if(flags & MAP_FIXED) {
        kprintf("Fixed\n");

        vmm_map_range(local->page_map, (uint64_t)addr, page_cnt, 0x3 | (1 << 2),  0x3 | (1 << 2)); 
        if(!(flags & MAP_ANONYMOUS)) 
            read(fd, addr, length);
    }

    return addr;
}

void syscall_mmap(struct regs *regs) {
    regs->rax = (size_t)mmap((void*)regs->rdi, regs->rsi, (int)regs->rdx, (int)regs->r10, (int)regs->r8, (int64_t)regs->r9);
}
