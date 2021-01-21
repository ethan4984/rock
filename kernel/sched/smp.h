#ifndef SMP_H_
#define SMP_H_

#include <int/apic.h>

#include <mm/vmm.h>
#include <mm/pmm.h>

typedef struct {
    uint64_t core_index;
    uint64_t kernel_stack;
    uint64_t user_stack;
    int pid;
    int tid;
} core_local_t;

core_local_t *get_core_local(int64_t index);

void init_smp();

void spin_lock(void *lock);

void spin_release(void *lock);

#endif
