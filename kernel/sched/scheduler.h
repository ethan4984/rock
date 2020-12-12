#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <mm/vmm.h>
#include <asmutils.h>
#include <memutils.h>

#define FSBASE_MSR 0xc0000100
#define GSBASE_MSR 0xc0000101

extern void start_task(uint64_t ss, uint64_t rsp, uint64_t cs, uint64_t rip);
extern void switch_task(uint64_t rsp, uint64_t ss);

enum {
    WAITING,
    WAITING_TO_START,
    RUNNING
};

typedef struct {
    int pid;
    int tid;
    int core_index;
} core_local_t;

typedef struct {
    int tid;
    int pid;
    int lock;
    int idle_cnt;
    int status;
    int exists;
    regs_t regs;
    uint64_t kernel_stack;
    uint64_t user_stack;
    uint64_t starting_addr;
    int ks_page_cnt, us_page_cnt;
} thread_t;

typedef struct {
    int pid;
    int max_thread_cnt;
    int file_handle_cnt;
    int idle_cnt;
    int status;
    int exists;
    pagestruct_t *page;
    thread_t *threads;
    int *file_handles;
    char lock;
} task_t;

void scheduler_init();

void scheduler_main(regs_t *regs);

int kill_task(int pid);

int create_task(uint64_t starting_addr);

int create_task_thread(int pid, uint64_t starting_addr);

int kill_thread(int pid, int tid); 

volatile core_local_t *get_core_local();

#endif
