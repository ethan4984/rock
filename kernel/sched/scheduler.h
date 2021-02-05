#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <mm/vmm.h>
#include <types.h>
#include <asmutils.h>
#include <memutils.h>
#include <vec.h>

extern void start_task(uint64_t ss, uint64_t rsp, uint64_t cs, uint64_t rip);
extern void switch_task(uint64_t rsp);
extern char sched_lock;

#define SCHED_WAITING 0
#define SCHED_WAITING_TO_START 1
#define SCHED_RUNNING 2

#define SCHED_USER (1 << 1)
#define SCHED_KERNEL (1 << 2)
#define SCHED_ELF (1 << 3)

typedef struct {
    tid_t tid;
    size_t idle_cnt, status;
    uint64_t user_stack;
    uint64_t kernel_stack;
    uint64_t user_gs_base;
    uint64_t user_fs_base;
    size_t user_stack_size;
    size_t kernel_stack_size;
    regs_t regs;
} thread_t;

typedef struct {
    pid_t pid;
    pid_t ppid;
    size_t idle_cnt, status;
    uninit_hash_table(thread_t, threads);
    uninit_vec(int, open_fds);
    pagestruct_t *pagestruct;
} task_t;

void scheduler_main(regs_t *regs);
task_t *sched_create_task(task_t *parent, pagestruct_t *pagestruct);
thread_t *sched_create_thread(pid_t pid, uint64_t starting_addr, uint16_t cs);
int sched_exec(char *path, char **argv, char **envp, int mode);

#endif
