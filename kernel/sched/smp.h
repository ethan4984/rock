#ifndef SMP_H_
#define SMP_H_

#include <int/apic.h>
#include <types.h>
#include <mm/vmm.h>
#include <mm/pmm.h>

#define CURRENT_CORE -1

typedef struct {
    uint64_t core_index;
    uint64_t kernel_stack;
    uint64_t user_stack;
    int errno;
    pid_t pid;
    tid_t tid;
    pagestruct_t *pagestruct;
} core_local_t;

core_local_t *get_core_local(int64_t index);
void init_smp();

void spin_lock(void *lock);
void spin_release(void *lock);

#endif
