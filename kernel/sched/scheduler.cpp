#include <sched/scheduler.hpp>
#include <sched/smp.hpp>
#include <int/apic.hpp>

namespace sched {

lib::map<ssize_t, task> task_list;

void reschedule(regs *regs_cur) {
    spin_lock(&scheduler_lock);

    if(regs_cur->cs & 0x3)
        swapgs();

    auto &&next_task = []() {
        task &&ret = task();

        for(size_t i = 0, cnt = 0; i < task_list.size(); i++) {
            task &next_task = task_list[task_list.get_tag(i)];
            next_task.idle_cnt++;

            if(next_task.status == task_waiting && cnt < next_task.idle_cnt) {
                cnt = next_task.idle_cnt;
                ret = next_task;
            }
        }
        
        return ret;
    } ();

    if(next_task.pid == -1) {
        if(regs_cur->cs & 0x3)
            swapgs();
        spin_release(&scheduler_lock);
        return;
    }

    auto &&next_thread = [&next_task]() {
        thread &&ret = thread();

        for(size_t i = 0, cnt = 0; i < next_task.threads.size(); i++) {
            thread &next_thread = next_task.threads[next_task.threads.get_tag(i)];
            next_thread.idle_cnt++;

            if(next_thread.status == task_waiting && cnt < next_thread.idle_cnt) {
                cnt = next_thread.idle_cnt;
                ret = next_thread;
            }
        }

        return ret;
    } ();

    if(next_thread.tid == -1) {
        if(regs_cur->cs & 0x3)
            swapgs();
        spin_release(&scheduler_lock);
        return;
    }

    ssize_t tid = next_thread.tid; 
    ssize_t pid = next_task.pid;

    smp::cpu &cpu_local = smp::core_local();

    if(cpu_local.tid != -1 && cpu_local.pid != -1) {
        task &last_task = task_list[cpu_local.pid];
        thread &last_thread = last_task.threads[cpu_local.tid];

        last_thread.status = task_waiting;
        last_task.status = task_waiting;

        last_thread.regs_cur = *regs_cur;
        last_thread.user_fs_base = get_user_fs();
        last_thread.user_gs_base = get_user_gs();
    }

    cpu_local.pid = pid;
    cpu_local.tid = tid;

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

    apic::lapic_write(apic::eoi, 0);
    spin_release(&scheduler_lock);
}

}
