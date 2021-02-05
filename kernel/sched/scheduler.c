#include <sched/scheduler.h>
#include <acpi/madt.h>
#include <vec.h>
#include <bitmap.h>
#include <output.h>
#include <sched/smp.h>

static_hash_table(task_t, tasks);
char sched_lock = 0;

static thread_t *find_next_thread(task_t *task) {
    thread_t *ret = NULL; 

    for(size_t i = 0, cnt = 0; i < task->threads.hash_cnt; i++) {
        thread_t *thread = vec_search(thread_t, task->threads.data_map, i);
        thread->idle_cnt++;

        if(thread->status == SCHED_WAITING && cnt < thread->idle_cnt) {
            cnt = thread->idle_cnt;
            ret = thread;
        }
    }

    return ret;
}

static task_t *find_next_task() {
    task_t *ret = NULL;

    if(tasks.hash_cnt == 1) {
        return vec_search(task_t, tasks.data_map, 0);
    }

    for(size_t i = 0, cnt = 0; i < tasks.hash_cnt; i++) {
        task_t *task = vec_search(task_t, tasks.data_map, i);
        task->idle_cnt++;

        if(task->status == SCHED_WAITING && cnt < task->idle_cnt) {
            cnt = task->idle_cnt;
            ret = task;
        }
    }

    return ret;
}

static void reschedule(regs_t *regs) {
    if(regs->cs & 0x3)
        swapgs();

    core_local_t *local = get_core_local(CURRENT_CORE);
    
    task_t *next_task = find_next_task();
    if(next_task == NULL) {
        if(regs->cs & 0x3)
            swapgs();
        return;
    }

    thread_t *next_thread = find_next_thread(next_task);
    if(next_thread == NULL) {
        if(regs->cs & 0x3)
            swapgs();
        return;
    }

    int tid = next_thread->tid;
    int pid = next_task->pid;

    if(local->tid != -1 && local->pid != -1) {
        task_t *old_task = hash_search(task_t, tasks, local->pid);
        if(old_task == NULL) {
            if(regs->cs & 0x3)
                swapgs();
            return;
        }

        thread_t *old_thread = hash_search(thread_t, old_task->threads, local->tid);
        if(old_thread == NULL) {
            if(regs->cs & 0x3)
                swapgs();
            return;
        }

        old_thread->regs = *regs;
        old_thread->status = SCHED_WAITING;
        old_task->status = SCHED_WAITING;

        old_thread->user_gs_base = get_user_gs(); 
        old_thread->user_fs_base = get_user_fs();
    }

    local->pid = pid;
    local->tid = tid;

    next_thread->idle_cnt = 0;
    next_task->idle_cnt = 0;
    
    next_thread->status = SCHED_RUNNING;
    next_task->status = SCHED_RUNNING;

    set_user_fs(next_thread->user_fs_base);
    set_user_gs(next_thread->user_gs_base);
        
    if(next_thread->regs.cs & 0x3)
        swapgs();
    
    lapic_write(LAPIC_EOI, 0);
    spin_release(&sched_lock);

    switch_task((uint64_t)&next_thread->regs);
}

task_t *sched_create_task(task_t *parent, pagestruct_t *pagestruct) {
    task_t task = { .status = SCHED_WAITING };

    if(parent != NULL) {
        task.ppid = parent->pid;
    }

    if(pagestruct != NULL) {
        task.pagestruct = pagestruct;
    }

    pid_t new_pid = hash_push(task_t, tasks, task);
    task_t *new_task = hash_search(task_t, tasks, new_pid);
    new_task->pid = new_pid;

    return new_task;
}

thread_t *sched_create_thread(pid_t pid, uint64_t starting_addr, uint16_t cs) {
    task_t *task = hash_search(task_t, tasks, pid);
    if(task == NULL)
        return NULL;

    thread_t thread = { .status = SCHED_WAITING,
                        .kernel_stack = pmm_alloc(4),
                        .user_stack = pmm_alloc(4), 
                        .kernel_stack_size = 4,
                        .user_stack_size = 4,
                      };

    tid_t new_tid = hash_push(thread_t, task->threads, thread);
    thread_t *new_thread = hash_search(thread_t, task->threads, new_tid);
    new_thread->tid = new_tid;

    new_thread->regs.rip = starting_addr;
    new_thread->regs.cs = cs;
    new_thread->regs.ss = cs - 8;
    new_thread->regs.rflags = 0x202;

    if(cs & 0x3) {
        new_thread->regs.rsp = new_thread->user_stack + HIGH_VMA;
    } else {
        new_thread->regs.rsp = new_thread->kernel_stack + HIGH_VMA;
    }

    return new_thread;
}

void scheduler_main(regs_t *regs) {
    spin_lock(&sched_lock);

    reschedule(regs);

    spin_release(&sched_lock);
}
