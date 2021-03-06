#include <mm/mmap.h> 
#include <sched/smp.h>
#include <fs/fd.h>
#include <debug.h>
#include <vec.h>

static void *mmap_alloc(struct page_map *page_map, void *addr, size_t length, int flags) {
    size_t page_cnt = DIV_ROUNDUP(length, PAGE_SIZE), offset = 0;

    if(page_map->bitmap == NULL) { 
        page_map->bm_size = 0x1000;
        page_map->bitmap = kcalloc(page_map->bm_size);
    }

    if(flags & MAP_FIXED) {
        size_t index = ((size_t)addr - MMAP_MIN_ADDR) / PAGE_SIZE;
        for(size_t i = index; i < index + page_cnt; i++) {
            BM_SET(page_map->bitmap, i);
        }
        return addr;
    }

    for(size_t i = 0, cnt = 0; i < page_map->bm_size; i++) {
        if(BM_TEST(page_map->bitmap, i)) {
            offset += cnt + 1;
            cnt = 0;
            continue;
        }

        if(++cnt == page_cnt) {
            addr = (void*)(offset * PAGE_SIZE + MMAP_MIN_ADDR);
            size_t index = ((size_t)addr - MMAP_MIN_ADDR) / PAGE_SIZE;

            for(uint64_t i = index; i < index + page_cnt; i++) {
                BM_SET(page_map->bitmap, i);
            }

            return addr;
        }
    }

    page_map->bm_size += 0x1000;
    page_map->bitmap = krecalloc(page_map->bitmap, page_map->bm_size);

    return mmap_alloc(page_map, addr, length, flags);
}

static int check_mmap_addr(struct page_map *page_map, int flags, void *addr, uint64_t length) {
    if(addr == NULL)
        return -1;

    if(flags == MAP_FIXED)
        return 0;

    if(page_map->bitmap == NULL) {
        page_map->bm_size = 0x1000;
        page_map->bitmap = kcalloc(page_map->bm_size);
    }

    size_t index = ((uint64_t)addr - MMAP_MIN_ADDR) / PAGE_SIZE;
    size_t page_cnt = DIV_ROUNDUP(length, PAGE_SIZE);

    for(uint64_t i = index; i < index + page_cnt; i++) { 
        if(BM_TEST(page_map->bitmap, i)) {
            return -1;
        }
    }
    
    return 0;
}

void *mmap(struct page_map *page_map, void *addr, size_t length, int prot, int flags, int fd, int64_t off) {
    size_t page_cnt = DIV_ROUNDUP(length, PAGE_SIZE);

    if(flags == MAP_ANONYMOUS) {
        if(check_mmap_addr(page_map, flags, addr, length) == -1) {
            addr = mmap_alloc(page_map, addr, length, flags);
        }
       
        vmm_map_range(page_map, (uint64_t)addr, page_cnt, prot, prot);
    } 

    if(flags & MAP_FIXED) {
        vmm_map_range(page_map, (uint64_t)addr, page_cnt, prot, prot); 
        mmap_alloc(page_map, addr, length, flags);
        if(!(flags & MAP_ANONYMOUS)) 
            read(fd, addr, length);
    }

    return addr;
}

int munmap(struct page_map *page_map, void *addr, size_t length) {
    size_t index = ((size_t)addr - MMAP_MIN_ADDR) / PAGE_SIZE;
    size_t page_cnt = DIV_ROUNDUP(length, PAGE_SIZE);

    if(index >= page_map->bm_size) {
        return -1;
    } else if(index + page_cnt > page_map->bm_size) {
        page_cnt = page_map->bm_size - index;
    }

    for(size_t i = index; i < index + page_cnt; i++) { 
        BM_CLEAR(page_map->bitmap, i);
    }

    vmm_unmap_range(page_map, (size_t)addr, page_cnt, 0);

    return 0;
}

void mmap_reserve(struct page_map *page_map, void *addr, size_t length) {
    if(page_map->bitmap == NULL) {
        page_map->bm_size = 0x1000;
        page_map->bitmap = kcalloc(page_map->bm_size);
    }
    
    size_t index = ((size_t)addr - MMAP_MIN_ADDR) / PAGE_SIZE;
    size_t page_cnt = DIV_ROUNDUP(length, PAGE_SIZE);

    if(index >= page_map->bm_size) {
        page_map->bm_size = ALIGN_UP(index, 0x1000);
        page_map->bitmap = krecalloc(page_map->bitmap, page_map->bm_size); 
    } else if(index + page_cnt > page_map->bm_size) {
        page_cnt = page_map->bm_size - index;
    }

    for(size_t i = index; i < index + page_cnt; i++) {
        BM_SET(page_map->bitmap, i);
    }
}

void syscall_mmap(struct regs *regs) {
    struct core_local *local = get_core_local(CURRENT_CORE);
    regs->rax = (size_t)mmap(local->page_map, (void*)regs->rdi, regs->rsi, (int)regs->rdx | (1 << 2), (int)regs->r10, (int)regs->r8, (int64_t)regs->r9);
}

void syscall_munmap(struct regs *regs) {
    struct core_local *local = get_core_local(CURRENT_CORE);
    regs->rax = (size_t)munmap(local->page_map, (void*)regs->rdi, regs->rsi);
}
