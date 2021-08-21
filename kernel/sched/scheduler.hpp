#ifndef SCHEDULER_HPP_
#define SCHEDULER_HPP_

#include <bitmap.hpp>
#include <fs/fd.hpp>
#include <types.hpp>
#include <map.hpp>
#include <cpu.hpp>
#include <elf.hpp>

namespace sched {

constexpr size_t task_waiting = (1 << 0);
constexpr size_t task_running = (1 << 1);
constexpr size_t task_user = (1 << 2);
constexpr size_t task_elf = (1 << 3);

constexpr size_t thread_stack_size = 0x20000;

extern "C" void switch_task(size_t rsp);

struct event {
    size_t tid;
    size_t pid;
    size_t pgid;
    size_t ppid;
    size_t type;
    size_t ret;
    size_t status;
    size_t lock;
};

class arguments {
public:
    arguments(const char **raw_argv, const char **raw_envp);

    uint64_t placement(uint64_t *ptr, elf::aux *aux);

    size_t envp_cnt;
    size_t argv_cnt;

    char **argv;
    char **envp;
private:
    const char **raw_argv;
    const char **raw_envp;
};

struct thread {
    thread(pid_t pid) : tid(-1), pid(pid), idle_cnt(0), errno(0) { }
    thread() : tid(-1), pid(-1), idle_cnt(0), errno(0) { }

    ssize_t exec(uint64_t rip, uint16_t cs, elf::aux *aux, arguments args);

    tid_t tid;
    pid_t pid;

    size_t status;
    size_t idle_cnt;
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
    task(ssize_t ppid, vmm::pmlx_table *page_map);
    task(ssize_t ppid);
    task() : pid(-1), ppid(-1), idle_cnt(0), working_directory(NULL) { }

    ssize_t exec(lib::string path, uint16_t cs, arguments args, vfs::node *working_dir, tid_t tid = -1);

    pid_t pid;
    pid_t ppid;

    size_t idle_cnt;
    size_t status;

    lib::map<ssize_t, thread*> threads;
    lib::bitmap tid_bitmap;
    lib::vector<event> event_list;

    vfs::node *working_directory;

    struct {
        lib::map<ssize_t, fs::fd> list;
        uint8_t *bitmap;
        size_t bitmap_size;
    } fd_list;

    vmm::pmlx_table *page_map;
};

void reschedule(regs*, void*);

inline lib::bitmap pid_bitmap(0x1000);
inline lib::map<ssize_t, task*> task_list;
inline size_t scheduler_lock = 0;

}

#endif
