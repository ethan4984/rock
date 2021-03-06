#include <sched/scheduler.hpp>
#include <sched/smp.hpp>
#include <int/apic.hpp>
#include <mm/mmap.hpp>
#include <fs/fd.hpp>

namespace sched {

static size_t thread_cnt = 0;
static size_t task_cnt = 0;

ssize_t create_task(ssize_t ppid, vmm::pmlx_table *page_map) {
    task new_task;

    if(task_list[ppid].pid != -1)
        new_task.ppid = ppid;
    else
        new_task.ppid = -1;

    new_task.status = task_waiting;
    new_task.ppid = ppid;

    if(page_map != NULL) {
        new_task.page_map = page_map;
    } else {
        new_task.page_map = vmm::kernel_mapping;
    } 

    task_list[new_task.pid = task_cnt++] = new_task;

    return new_task.pid;
}

ssize_t create_thread(ssize_t pid, uint64_t rip, uint16_t cs, elf::aux *aux, const char **argv, const char **envp) {
    if(task_list[pid].pid == -1)
        return -1;

    thread new_thread;

    new_thread.regs_cur.rip = rip;
    new_thread.regs_cur.cs = cs;
    new_thread.regs_cur.rflags = 0x202;

    new_thread.status = task_waiting;
    new_thread.kernel_stack = pmm::alloc(2);
    new_thread.user_gs_base = 0;
    new_thread.user_fs_base = 0;
    new_thread.user_stack = 0;

    if(cs & 0x3) {
        new_thread.regs_cur.ss = cs - 8;

        new_thread.user_stack = (size_t)mm::mmap(task_list[pid].page_map, NULL, thread_stack_size + 0x1000, 0x3 | (1 << 2), mm::map_anonymous, 0, 0) + thread_stack_size;
        new_thread.regs_cur.rsp = new_thread.user_stack;

        uint64_t *stack = (uint64_t*)new_thread.regs_cur.rsp;

        size_t envp_cnt = 0;
        size_t argv_cnt = 0;

        while(*envp) {
            const char *element = *envp;
            stack = (uint64_t*)((size_t)stack - strlen(element) + 1);
            strcpy((char*)stack, element);
            envp++; envp_cnt++;
        }

        while(*argv) {
            const char *element = *argv;
            stack = (uint64_t*)((size_t)stack - strlen(element) + 1);
            strcpy((char*)stack, element);
            argv++; argv_cnt++;
        }

        argv -= argv_cnt; envp -= envp_cnt;

        stack = (uint64_t*)((size_t)stack - ((size_t)stack & 0xf));

        if((argv_cnt + envp_cnt + 1) & 1)
            stack--;

        stack -= 10;

        stack[0] = elf::at_phnum; stack[1] = aux->at_phnum;
        stack[2] = elf::at_phent; stack[3] = aux->at_phent;
        stack[4] = elf::at_phdr;  stack[5] = aux->at_phdr;
        stack[6] = elf::at_entry; stack[7] = aux->at_entry;
        stack[8] = 0; stack[9] = 0;

        uint64_t save = new_thread.regs_cur.rsp;

        *(--stack) = 0;
        stack -= envp_cnt;

        for(size_t i = 0; i < envp_cnt; i++) {
            save -= strlen(envp[i]) + 1;
            stack[i] = save;
        }

        *(--stack) = 0;
        stack -= argv_cnt;

        for(size_t i = 0; i < argv_cnt; i++) {
            save -= strlen(argv[i]) + 1;
            stack[i] = save;
        }

        *(--stack) = argv_cnt; // argc

        new_thread.regs_cur.rsp = (uint64_t)stack;
    } else {
        new_thread.regs_cur.ss = cs + 8;
        new_thread.regs_cur.rsp = new_thread.kernel_stack + vmm::high_vma;
    }

    task_list[pid].threads[new_thread.tid = thread_cnt++] = new_thread;

    return new_thread.tid;
}

void reschedule(regs *regs_cur) {
    spin_lock(&scheduler_lock);

    if(regs_cur->cs & 0x3)
        swapgs();

    auto next_pid = []() {
        ssize_t ret = -1;

        for(size_t i = 0, cnt = 0; i < task_list.size(); i++) {
            task &next_task = task_list[task_list.get_tag(i)];
            next_task.idle_cnt++;

            if(next_task.status == task_waiting && cnt < next_task.idle_cnt) {
                cnt = next_task.idle_cnt;
                ret = next_task.pid;
            }
        }
        
        return ret;
    } ();

    smp::cpu &cpu_local = smp::core_local();

    if(next_pid == -1) {
        if(cpu_local.pid != -1) {
            next_pid = cpu_local.pid;
        } else {
            if(regs_cur->cs & 0x3)
                swapgs();
            spin_release(&scheduler_lock);
            return;
        }
    }

    task &next_task = task_list[next_pid];

    auto next_tid = [&]() {
        ssize_t ret = -1;

        for(size_t i = 0, cnt = 0; i < next_task.threads.size(); i++) {
            thread &next_thread = next_task.threads[next_task.threads.get_tag(i)];
            next_thread.idle_cnt++;

            if(next_thread.status == task_waiting && cnt < next_thread.idle_cnt) {
                cnt = next_thread.idle_cnt;
                ret = next_thread.tid;
            }
        }

        return ret;
    } ();

    if(next_tid == -1) {
        if(regs_cur->cs & 0x3)
            swapgs();
        spin_release(&scheduler_lock);
        return;
    }

    thread &next_thread = task_list[next_pid].threads[next_tid];

    if(cpu_local.tid != -1 && cpu_local.pid != -1) {
        task &last_task = task_list[cpu_local.pid];
        thread &last_thread = last_task.threads[cpu_local.tid];

        last_thread.status = task_waiting;
        last_task.status = task_waiting;

        last_thread.regs_cur = *regs_cur;
        last_thread.user_fs_base = get_user_fs();
        last_thread.user_gs_base = get_user_gs(); 
    }

    cpu_local.pid = next_pid;
    cpu_local.tid = next_tid;
    cpu_local.errno = next_thread.errno;

    cpu_local.page_map = next_task.page_map;
    cpu_local.page_map->init();

    next_thread.idle_cnt = 0;
    next_task.idle_cnt = 0;

    next_thread.status = task_running;
    next_task.status = task_running;

    set_user_fs(next_thread.user_fs_base);
    set_user_gs(next_thread.user_gs_base);

    if(next_thread.regs_cur.cs & 0x3)
        swapgs();

    apic::lapic->write(apic::lapic->eoi(), 0);
    spin_release(&scheduler_lock);

    switch_task((uint64_t)&task_list[next_pid].threads[next_tid].regs_cur);
}

ssize_t sched_task(lib::string path, uint16_t cs, const char **argv, const char **envp) { 
    fs::fd file(path, 0, 0);
    if(file.status == 0)
        return -1;

    smp::cpu &core = smp::core_local();

    ssize_t ppid = core.pid;

    asm ("cli");

    vmm::pmlx_table *page_map = core.page_map->create_generic();
    page_map->init();

    lib::string *ld_path = NULL;

    elf::aux aux;
    elf::file(page_map, &aux, file, 0, &ld_path);

    uint64_t entry_point = aux.at_entry;

    if(ld_path != NULL) {
        fs::fd ld_file(*ld_path, 0, 0);
        if(ld_file.status == 0)
            return -1;

        elf::aux ld_aux;
        elf::file(page_map, &ld_aux, ld_file, 0x40000000, NULL);

        entry_point = ld_aux.at_entry;
    }

    ssize_t pid = create_task(ppid, page_map);
    create_thread(pid, entry_point, cs, &aux, argv, envp);

    core.page_map->init();

    asm ("sti");

    return 0;
}

}
