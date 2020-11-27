#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <mm/vmm.h>
#include <asmutils.h>
#include <memutils.h>

#define FSBASE_MSR 0xc0000100
#define GSBASE_MSR 0xc0000101

extern void start_task();
extern void switch_task();

typedef struct {
    int current_pid;
    int current_tid;
} core_local_t;

typedef struct {
    int tid;
    int pid;
    int lock;
    uint64_t kernel_stack;
    uint64_t user_stack;
} thread_t;

typedef struct {
    int pid;
    int ppid;
    pagestruct_t *page;
    thread_t *threads;
    int *file_handles;
    char lock;
} task_t;

void scheduler_init();

void scheduler_main(regs_t *regs);

#endif
