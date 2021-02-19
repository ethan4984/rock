#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <mm/vmm.h>
#include <types.h>
#include <cpu.h>
#include <memutils.h>
#include <vec.h>
#include <elf.h>

extern void start_task(uint64_t ss, uint64_t rsp, uint64_t cs, uint64_t rip);
extern void switch_task(uint64_t rsp);
extern char sched_lock;

#define SCHED_WAITING 0
#define SCHED_WAITING_TO_START 1
#define SCHED_RUNNING 2

#define SCHED_USER (1 << 1)
#define SCHED_KERNEL (1 << 2)
#define SCHED_ELF (1 << 3)

struct thread {
    tid_t tid;
    size_t idle_cnt, status;
    uint64_t user_stack;
    uint64_t kernel_stack;
    uint64_t user_gs_base;
    uint64_t user_fs_base;
    size_t user_stack_size;
    size_t kernel_stack_size;
    struct regs regs;
};

struct task {
    pid_t pid;
    pid_t ppid;
    size_t idle_cnt, status;
    uninit_hash_table(struct thread, threads);
    uninit_vec(int, fd_list);
    struct page_map *page_map;
    struct vfs_node *working_dir;
};

extern_hash_table(struct task, tasks);

void scheduler_main(struct regs *regs);
struct task *sched_create_task(struct task *parent, struct page_map *page_map);
struct thread *sched_create_thread(pid_t pid, struct aux *aux, const char **angv, const char **envp, uint64_t starting_addr, uint16_t cs);
int sched_exec(char *path, const char **argv, const char **envp, int mode);

#endif
