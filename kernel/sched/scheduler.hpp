#ifndef SCHEDULER_HPP_
#define SCHEDULER_HPP_

#include <types.hpp>
#include <map.hpp>
#include <cpu.hpp>
#include <elf.hpp>

namespace sched {

constexpr size_t task_waiting = (1 << 1);
constexpr size_t task_waiting_to_start = (1 << 2);
constexpr size_t task_running = (1 << 3);

constexpr size_t task_user = (1 << 4);
constexpr size_t task_elf = (1 << 5);

constexpr size_t thread_stack_size = 0x2000;

extern "C" void switch_task(uint64_t rsp);

struct thread {
    thread() : tid(-1), idle_cnt(0), errno(0) { }

    tid_t tid;
    size_t idle_cnt;
    size_t status;
    size_t user_stack;
    size_t kernel_stack;
    size_t user_gs_base;
    size_t user_fs_base;
    size_t user_stack_size;
    size_t kernel_stack_size;
    size_t errno;
    regs regs_cur;
};

struct task {
    task() : pid(-1), ppid(-1), idle_cnt(0) { }
  
    pid_t pid;
    pid_t ppid;
    size_t idle_cnt;
    size_t status;
    lib::map<ssize_t, thread> threads;

    struct {
        uint8_t *bitmap;
        size_t bitmap_size;
    } fd_list;

    size_t file_descriptor_bitmap_size;
    vmm::pmlx_table *page_map;
};

ssize_t create_task(ssize_t pid, vmm::pmlx_table *page_map);
ssize_t create_thread(ssize_t ppid, uint64_t rip, uint16_t cs, elf::aux *aux, const char **argv, const char **envp);
ssize_t sched_task(lib::string path, uint16_t cs, const char **argv, const char **envp);
void reschedule(regs *regs_cur);

inline size_t scheduler_lock = 0;
inline lib::map<ssize_t, task> task_list;

}

#endif
