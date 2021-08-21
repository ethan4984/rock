#include <sched/scheduler.hpp>
#include <sched/smp.hpp>
#include <int/apic.hpp>
#include <mm/mmap.hpp>
#include <fs/fd.hpp>
#include <drivers/tty.hpp>

namespace sched {

arguments::arguments(const char **raw_argv, const char **raw_envp) : raw_argv(raw_argv), raw_envp(raw_envp) {
    auto arg_cnt = [](const char **arg) -> size_t {
        size_t cnt = 0;
        while(*arg) {
            cnt++;
            arg++;
        }
        return cnt;
    };

    auto copy = [](char **dest, const char **src, size_t cnt) -> void {
        size_t i = 0;
        for(; i < cnt; i++) {
            char *element = new char[strlen(src[i])] + 1;
            strcpy(element, src[i]);
            dest[i] = element;
        }

        dest[i] = NULL;
    };

    argv_cnt = arg_cnt(raw_argv);
    envp_cnt = arg_cnt(raw_envp);

    argv = new char*[argv_cnt + 1];
    envp = new char*[envp_cnt + 1];

    copy(argv, raw_argv, argv_cnt);
    copy(envp, raw_envp, envp_cnt);
}

uint64_t arguments::placement(uint64_t *ptr, elf::aux *aux) {
    auto copy = [&, this](char **arg, size_t cnt) -> void {
        for(size_t i = 0; i < cnt; i++) {
            char *element = arg[i];
            ptr = (uint64_t*)((uint8_t*)ptr - (strlen(element) + 1));
            strcpy((char*)ptr, element);
        }
    };

    uint64_t save = (uint64_t)ptr;

    copy(envp, envp_cnt);
    copy(argv, argv_cnt);

    ptr = (uint64_t*)((uint8_t*)ptr - ((uint64_t)ptr & 0xf)); // ensure alignment

    if((argv_cnt + envp_cnt + 1) & 1)
        ptr--;

    ptr -= 10;

    ptr[0] = elf::at_phnum; ptr[1] = aux->at_phnum;
    ptr[2] = elf::at_phent; ptr[3] = aux->at_phent;
    ptr[4] = elf::at_phdr;  ptr[5] = aux->at_phdr;
    ptr[6] = elf::at_entry; ptr[7] = aux->at_entry;
    ptr[8] = 0; ptr[9] = 0;

    *(--ptr) = 0;
    ptr -= envp_cnt;

    for(size_t i = 0; i < envp_cnt; i++) {
        save -= strlen(envp[i]) + 1;
        ptr[i] = save;
    }

    *(--ptr) = 0;
    ptr -= argv_cnt;

    for(size_t i = 0; i < argv_cnt; i++) {
        save -= strlen(argv[i]) + 1;
        ptr[i] = save;
    }

    *(--ptr) = argv_cnt;

    return (uint64_t)ptr;
}

ssize_t thread::exec(uint64_t rip, uint16_t cs, elf::aux *aux, arguments args) {
    if(pid == -1)
        return -1;

    regs_cur.rip = rip;
    regs_cur.cs = cs;
    regs_cur.rflags = 0x202;

    status = task_waiting;
    kernel_stack = pmm::alloc(4) + 0x4000 + vmm::high_vma;
    user_gs_base = 0;
    user_gs_base = 0;
    idle_cnt = 0;

    if(cs & 0x3) {
        regs_cur.ss = cs - 8;
        user_stack = (size_t)mm::mmap(task_list[pid]->page_map, NULL, thread_stack_size + 0x1000, 0x3 | (1 << 2), mm::map_anonymous, 0, 0) + thread_stack_size;
        regs_cur.rsp = args.placement((uint64_t*)user_stack, aux);
    } else {
        regs_cur.ss = cs + 8;
        regs_cur.rsp = kernel_stack;
    }

    if(tid == -1) {
        tid = task_list[pid]->tid_bitmap.alloc(); 
        task_list[pid]->threads[tid] = this;
    }

    return tid;
}

task::task(ssize_t ppid, vmm::pmlx_table *page_map) : ppid(ppid), idle_cnt(0), page_map(page_map) {
    status = task_waiting;
    fd_list.bitmap_size = 0x1000;
    fd_list.bitmap = (uint8_t*)kmm::calloc(0x1000 / 8);

    if(page_map == NULL) 
        page_map = vmm::kernel_mapping;

    pid = pid_bitmap.alloc();
    tid_bitmap = lib::bitmap(0x1000);

    task_list[pid] = this;
}

task::task(ssize_t ppid) : ppid(ppid), idle_cnt(0) {
    status = task_waiting;
    fd_list.bitmap_size = 0x1000;
    fd_list.bitmap = (uint8_t*)kmm::calloc(0x1000 / 8);

    if(page_map == NULL) 
        page_map = vmm::kernel_mapping;

    pid = pid_bitmap.alloc();
    tid_bitmap = lib::bitmap(0x1000);

    task_list[pid] = this;
}

ssize_t task::exec(lib::string path, uint16_t cs, arguments args, vfs::node *working_dir, tid_t tid) {
    smp::cpu *core = smp::core_local();

    page_map = core->page_map->create_generic();
    page_map->init();

    fs::fd file(path, s_irusr, 0);
    if(file.status == 0) {
        spin_release(&scheduler_lock);
        return -1;
    }

    lib::string *ld_path = NULL;

    elf::aux aux;
    if(elf::file(page_map, &aux, file, 0, &ld_path).status == 0) {
        return -1;
    }

    uint64_t entry_point = aux.at_entry;

    if(ld_path != NULL) {
        fs::fd ld_file(*ld_path, s_irusr, 0);
        if(ld_file.status == 0) {
            return -1;
        } 

        elf::aux ld_aux;
        if(elf::file(page_map, &ld_aux, ld_file, 0x40000000, NULL).status == 0) {
            return -1;
        }

        entry_point = ld_aux.at_entry;
    }

    if(tid == -1) {
        thread *new_thread = new thread(pid);
        new_thread->exec(entry_point, cs, &aux, args);
    } else {
        threads[tid]->exec(entry_point, cs, &aux, args);
    }

    working_directory = working_dir;

    size_t save_pid = core->pid;
    core->pid = pid;

    fs::alloc_fd(lib::string("/dev/tty"), s_irusr | s_iwusr);
    fs::alloc_fd(lib::string("/dev/tty"), s_iwusr | s_irusr);
    fs::alloc_fd(lib::string("/dev/tty"), s_irusr | s_iwusr);

    core->pid = save_pid;
    core->page_map->init();

    return 0;
}

void reschedule(regs *regs_cur, void*) {
    asm volatile ("" ::: "memory");

    if(__atomic_test_and_set(&scheduler_lock, __ATOMIC_ACQUIRE)) {
        return;
    }

    if(regs_cur->cs & 0x3)
        swapgs();

    auto next_pid = []() {
        ssize_t ret = -1;

        for(size_t i = 0, cnt = 0; i < task_list.size(); i++) {
            task *next_task = task_list[task_list.get_tag(i)];
            next_task->idle_cnt++;

            if(next_task->status == task_waiting && cnt < next_task->idle_cnt) {
                cnt = next_task->idle_cnt;
                ret = next_task->pid;
            }
        }
        
        return ret;
    } ();

    smp::cpu *cpu_local = smp::core_local();

    if(next_pid == -1) {
        if(cpu_local->pid != -1) {
            next_pid = cpu_local->pid;
        } else {
            if(regs_cur->cs & 0x3)
                swapgs();
            spin_release(&scheduler_lock);
            return;
        }
    }
    
    task *next_task = task_list[next_pid];

    auto next_tid = [&]() {
        ssize_t ret = -1;

        for(size_t i = 0, cnt = 0; i < next_task->threads.size(); i++) {
            thread *next_thread = next_task->threads[next_task->threads.get_tag(i)];
            next_thread->idle_cnt++;

            if(next_thread->status == task_waiting && cnt < next_thread->idle_cnt) {
                cnt = next_thread->idle_cnt;
                ret = next_thread->tid;
            }
        }

        return ret;
    } ();

    if(next_tid == -1) {
        if(cpu_local->pid != -1) {
            next_tid = cpu_local->tid; 
        } else {
            if(regs_cur->cs & 0x3)
                swapgs();
            spin_release(&scheduler_lock);
            return;
        }
    }

    thread *next_thread = task_list[next_pid]->threads[next_tid];

    if(cpu_local->tid != -1 || cpu_local->pid != -1) {
        task *last_task = task_list[cpu_local->pid];
        thread *last_thread = last_task->threads[cpu_local->tid];

        last_thread->status = task_waiting;
        last_task->status = task_waiting;

        last_thread->errno = cpu_local->errno;
        last_thread->regs_cur = *regs_cur;
        last_thread->user_fs_base = get_user_fs();
        last_thread->user_gs_base = get_user_gs();
        last_thread->user_stack = cpu_local->user_stack;
    }

    cpu_local->pid = next_pid;
    cpu_local->tid = next_tid;
    cpu_local->errno = next_thread->errno;
    cpu_local->kernel_stack = next_thread->kernel_stack;
    cpu_local->user_stack = next_thread->user_stack;

    cpu_local->page_map = next_task->page_map;
    cpu_local->page_map->init();

    next_thread->idle_cnt = 0;
    next_task->idle_cnt = 0;

    next_thread->status = task_running;
    next_task->status = task_running;

    set_user_fs(next_thread->user_fs_base);
    set_user_gs(next_thread->user_gs_base);

    if(next_thread->regs_cur.cs & 0x3)
        swapgs();

    apic::lapic->write(apic::lapic->eoi(), 0);
    spin_release(&scheduler_lock);

    switch_task((uint64_t)&task_list[next_pid]->threads[next_tid]->regs_cur);
}

extern "C" void syscall_getpid(regs *regs_cur) {
    spin_lock(&scheduler_lock);
    smp::cpu *core = smp::core_local();
    regs_cur->rax = core->pid;
    spin_release(&scheduler_lock);
}

extern "C" void syscall_gettid(regs *regs_cur) {
    spin_lock(&scheduler_lock);
    smp::cpu *core = smp::core_local();
    regs_cur->rax = core->tid;
    spin_release(&scheduler_lock);
}

extern "C" void syscall_getppid(regs *regs_cur) {
    spin_lock(&scheduler_lock);
    smp::cpu *core = smp::core_local();
    sched::task *current_task = sched::task_list[core->pid];
    regs_cur->rax = current_task->ppid;
    spin_release(&scheduler_lock);
}

extern "C" void syscall_exit(regs *regs_cur) {
    smp::cpu *core = smp::core_local();
    sched::task *current_task = sched::task_list[core->pid];

    for(size_t i = 0; i < current_task->fd_list.bitmap_size; i++) {
        current_task->fd_list.list.remove(bm_test(current_task->fd_list.bitmap, i));
    }

    for(size_t i = 0; i < current_task->threads.size(); i++) {
        current_task->threads.remove(current_task->threads.get_tag(i));
    }

    if(current_task->ppid != -1) {
        sched::task *parent_process = sched::task_list[current_task->ppid];

        event new_event {};

        new_event.pid = core->pid;
        new_event.tid = core->tid;
        new_event.ppid = parent_process->ppid;
        new_event.type = wifexited; 
        new_event.ret = core->pid;
        new_event.status = regs_cur->rdi;

        parent_process->event_list.push(new_event);
    }

    sched::task_list.remove(core->pid);
    pid_bitmap.free(core->pid);

    core->pid = -1;
    core->tid = -1;

    swapgs();
    for(;;) {
        reschedule(regs_cur, NULL);
    }
}

extern "C" void syscall_fork(regs *regs_cur) {
    smp::cpu *core = smp::core_local();
    sched::task *current_task = sched::task_list[core->pid];
    sched::thread *current_thread = current_task->threads[core->tid];

    vmm::pmlx_table *page_table = core->page_map->duplicate();

    task *new_task = new task(core->pid, page_table);
    thread *new_thread = new thread;

    new_thread->status = task_waiting;
    new_thread->regs_cur = *regs_cur;
    new_thread->regs_cur.rax = 0; 
    new_thread->user_gs_base = current_thread->user_gs_base;
    new_thread->user_fs_base = current_thread->user_fs_base;
    new_thread->kernel_stack = pmm::alloc(4) + 0x4000 + vmm::high_vma;
    new_thread->tid = new_task->tid_bitmap.alloc();
    new_thread->pid = new_task->pid;

    new_task->working_directory = current_task->working_directory;

    task_list[new_task->pid]->threads[new_thread->tid] = new_thread;
    task_list[new_task->pid]->fd_list = current_task->fd_list;

    regs_cur->rax = new_task->pid;
}

extern "C" void syscall_execve(regs *regs_cur) {
    spin_lock(&scheduler_lock);

    smp::cpu *core = smp::core_local();
    sched::task *current_task = sched::task_list[core->pid];
    sched::thread *current_thread = current_task->threads[core->tid];

    const char *path = (const char*)regs_cur->rdi;
    const char **argv = (const char**)regs_cur->rsi;
    const char **envp = (const char**)regs_cur->rdx;

    current_task->status = task_waiting;
    current_task->idle_cnt = 0;

    vfs::node *path_vfs_node = vfs::root_cluster->search_absolute(path);
    if(path_vfs_node == NULL) { 
        set_errno(enoent);
        regs_cur->rax = -1;
        return;
    }

    for(size_t i = 0; i < current_task->fd_list.bitmap_size; i++) {
        current_task->fd_list.list.remove(bm_test(current_task->fd_list.bitmap, i));
        bm_clear(current_task->fd_list.bitmap, i); 
    }

    arguments args(argv, envp);

    current_task->exec(lib::string(path), 0x23, args, path_vfs_node->parent, current_thread->tid);

    core->pid = -1;
    core->tid = -1;

    spin_release(&scheduler_lock);

    swapgs();
    for(;;) {
        reschedule(regs_cur, NULL);
    }
}

extern "C" void syscall_waitpid(regs *regs_cur) {
    int pid = regs_cur->rdi;
    int *wstatus = (int*)regs_cur->rsi;

    smp::cpu *core = smp::core_local();
    sched::task *current_task = sched::task_list[core->pid];

    auto poll = [](task *current_task, const auto &condition) -> size_t {
        for(;;) {
            for(size_t i = 0; i < current_task->event_list.size(); i++) {
                spin_lock(&current_task->event_list[i].lock);
                if(condition(current_task->event_list[i])) {
                    size_t ret = current_task->event_list[i].ret;
                    current_task->event_list.remove(i);
                    return ret;
                }
                spin_release(&current_task->event_list[i].lock);
            }
        }
    };

    if(pid < -1) {

    } else if(pid == -1) {
        const auto condition = [](event &event_cur) -> bool {
            if(event_cur.type == wifexited)
                return true;
            return false;
        };

        regs_cur->rax = poll(current_task, condition);

        *wstatus |= 0x7f | 0x200;
    } else if(pid == 0) {

    } else if(pid > 0) {
        const auto condition = [&](event &event_cur) -> bool {
            if(event_cur.type == wifexited && event_cur.pid == pid)
                return true;
            return false;
        };

        regs_cur->rax = poll(current_task, condition);

        *wstatus |= 0x7f | 0x200;
    }
}

}
