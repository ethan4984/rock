#include <mm/mmap.h>
#include <sched/scheduler.h>
#include <mm/vmm.h>
#include <fs/fd.h>
#include <stddef.h>

static void mmap_alloc(pagestruct_t *pagestruct, void *addr, uint64_t length) {
    uint64_t index = ((uint64_t)addr - MMAP_MIN_ADDR) / 0x1000;
    if(index < (pagestruct->bm_size * 8)) {
        pagestruct->bm_size += 0x1000;
        pagestruct->bitmap = krecalloc(pagestruct->bitmap, pagestruct->bm_size);
    }

    BM_SET(pagestruct->bitmap, index);
}

static void mmap_free(pagestruct_t *pagestruct, void *addr, uint64_t length) {
    uint64_t index = ((uint64_t)addr - MMAP_MIN_ADDR) / 0x1000;
    if(index < (pagestruct->bm_size * 8)) {
        return;
    }

    BM_CLEAR(pagestruct->bitmap, index);
}

static void *find_mmap_addr(pagestruct_t *pagestruct, uint64_t length) {
    uint64_t page_cnt = ROUNDUP(length, 0x1000), offset = 0;

    for(uint64_t i = 0, cnt = 0; i < pagestruct->bm_size; i++) {
        if(BM_TEST(pagestruct->bitmap, i)) {
            offset += cnt + 1;
            cnt = 0;
            continue;
        }

        if(++cnt == length) {
            void *ret = (void*)(offset * 0x1000 + MMAP_MIN_ADDR);
            mmap_alloc(pagestruct, ret, page_cnt);
            return ret;
        }
    }
    return find_mmap_addr(pagestruct, length);
}

static int check_mmap_addr(pagestruct_t *pagestruct, void *addr, uint64_t length) {
    if(addr == NULL)
        return -1;

    if((uint64_t)addr % 0x1000 != 0) 
        return -1;

    uint64_t index = ((uint64_t)addr - MMAP_MIN_ADDR) / 0x1000;
    for(uint64_t i = index; i < index + ROUNDUP(length, 0x1000); i++) { 
        if(BM_TEST(pagestruct->bitmap, i)) {
            return -1;
        }
    }
    
    return 0;
}

void *mmap(void *addr, uint64_t length, int prot, int flags, int fd, int64_t off) {
    task_t *current_task = get_current_task();
    if(current_task == NULL) {
        return (void*)-1;
    }

    if(check_mmap_addr(current_task->pagestruct, addr, length) == -1) {
        addr = find_mmap_addr(current_task->pagestruct, length);
    }

    map_range(current_task->pagestruct, (uint64_t)addr, ROUNDUP(length, 0x1000), flags); 

    read(fd, addr, length);

    return addr;
}

int munmap(void *addr, uint64_t length) {
    task_t *current_task = get_current_task();
    if(current_task == NULL) {
        return -1;
    }

    unmap_range(current_task->pagestruct, (uint64_t)addr, ROUNDUP(length, 0x1000), 0); // TODO no longer assumes 4kb pages

    mmap_free(current_task->pagestruct, addr, length);    

    return 0;
}
