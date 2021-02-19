#ifndef SMP_H_
#define SMP_H_

#include <types.h>
#include <mm/vmm.h>
#include <mm/pmm.h>

#define CURRENT_CORE -1

struct core_local {
    uint64_t core_index;
    uint64_t kernel_stack;
    uint64_t user_stack;
    int errno;
    pid_t pid;
    tid_t tid;
    struct page_map *page_map;
};

struct core_local *get_core_local(int index);
void smp_init();

void spin_lock(void *lock);
void spin_release(void *lock);

#endif
