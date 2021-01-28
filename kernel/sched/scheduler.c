#include <sched/scheduler.h>
#include <acpi/madt.h>
#include <vec.h>
#include <bitmap.h>
#include <output.h>
#include <sched/smp.h>

static int ready = 0, total_tasks = 0;
static task_t *tasks;
static char scheduler_lock = 0;

static void reschedule(regs_t *regs) {
    core_local_t *local = get_core_local(-1);

    task_t *next_task = NULL;
    int pid = -1, tid = -1;

    for(int i = 0, cnt = 0; i < total_tasks; i++) {
        next_task = ddl_get_index(task_t, tasks, i);

        if((next_task->status == WAITING) && (cnt < ++next_task->idle_cnt)) {
            cnt = tasks->idle_cnt;
            pid = i;
            continue;
        }

        if(next_task->status == WAITING_TO_START) {
            pid = i;
            break;
        }
    }

    if(next_task == NULL) {
        return; 
    }

    next_task->pid = pid;
    thread_t *next_thread = NULL;

    for(int i = 0, cnt = 0; i < next_task->thread_cnt; i++) {
        next_thread = ddl_get_index(thread_t, next_task->threads, i);

        if((next_thread->status == WAITING) && (cnt < ++next_thread->idle_cnt)) {
            cnt = next_thread->idle_cnt;
            tid = i;
            continue; 
        }

        if(next_thread->status == WAITING_TO_START) {
            tid = i;
            break;
        }
    }

    if(tid == -1) {
        return;
    }

    if((local->tid != -1) && (local->pid != -1)) {
        tasks[local->pid].threads[local->tid].regs = *regs;
        tasks[local->pid].threads[local->tid].status = WAITING;
        tasks[local->pid].status = WAITING;
    }

    local->pid = pid;
    local->tid = tid;

    next_thread->idle_cnt = 0;

    switch(next_thread->status) {
        case WAITING:
            next_thread->status = RUNNING;

            local->kernel_stack = next_thread->kernel_stack;

            lapic_write(LAPIC_EOI, 0); 
            spin_release(&scheduler_lock);

            switch_task((uint64_t)&next_thread->regs, next_thread->regs.ss);
            break; 
        case WAITING_TO_START:
            next_thread->status = RUNNING;

            local->kernel_stack = next_thread->kernel_stack;

            lapic_write(LAPIC_EOI, 0); 
            spin_release(&scheduler_lock);

            start_task(next_thread->regs.ss, next_thread->regs.rsp, next_thread->regs.cs, next_thread->starting_addr);
            break;
        default:
            kprintf("[KDEBUG]", "Invalid thread %d/%d status = %d", pid, tid, next_thread->status);
    }
}

void scheduler_main(regs_t *regs) {
    spin_lock(&scheduler_lock);

    if(!ready) { 
        spin_release(&scheduler_lock);
        return;                 
    }

    reschedule(regs);

    lapic_write(LAPIC_EOI, 0); 
    spin_release(&scheduler_lock);
}

int create_task(uint64_t start_addr, uint16_t cs) { 
    spin_lock(&scheduler_lock);

    static int pid_cnt = 0;

    task_t *new_task = kmalloc(sizeof(task_t));
    *new_task = (task_t) {  .cs = cs,
                            .ss = cs + 8,
                            .threads = kmalloc(sizeof(thread_t)),
                            .status = WAITING_TO_START,
                            .pid = pid_cnt++ // TODO limit of 2 billion :|
                         };

    ddl_push(task_t, tasks, new_task);

    create_task_thread(new_task->pid, start_addr);

    spin_release(&scheduler_lock);

    return new_task->pid;
}

int create_task_thread(int pid, uint64_t starting) {
    spin_lock(&scheduler_lock);

    task_t *task = ddl_search(task_t, tasks, pid, pid);
    if(task == NULL) 
        return -1;

    thread_t *new_thread = kmalloc(sizeof(thread_t));
    *new_thread = (thread_t) {  .pid = pid,
                                .status = WAITING_TO_START,
                                .kernel_stack = pmm_alloc(4),
                                .user_stack = pmm_alloc(4),
                                .ks_page_cnt = 4, 
                                .us_page_cnt = 4,
                                .starting_addr = starting,
                                .tid = task->tid_cnt++ // TODO limit of 2 billion :|
                             };

    ddl_push(thread_t, task->threads, new_thread);

    spin_release(&scheduler_lock);

    return new_thread->tid;
}

int kill_thread(int pid, int tid) {
    spin_lock(&scheduler_lock);

    task_t *task = ddl_search(task_t, tasks, pid, pid);
    if(task == NULL) 
        return -1;

    thread_t *thread = ddl_search(thread_t, task->threads, tid, tid);
    if(thread == NULL)
        return -1;

    pmm_free(thread->kernel_stack, thread->ks_page_cnt);
    pmm_free(thread->user_stack, thread->us_page_cnt);
   
    spin_release(&scheduler_lock);

    return 0;
}

int kill_task(int pid) {
    spin_lock(&scheduler_lock);

    task_t *task = ddl_search(task_t, tasks, pid, pid);
    if(task == NULL) 
        return -1;

    for(int i = 0; i < task->thread_cnt; i++) {
        thread_t *thread = ddl_get_index(thread_t, task->threads, i);
        pmm_free(thread->kernel_stack, thread->ks_page_cnt);
        pmm_free(thread->user_stack, thread->us_page_cnt);
    }

    kfree(task->threads);
    kfree(task->file_handles);

    spin_release(&scheduler_lock);

    return 0;
}

void scheduler_init() {
    tasks = kmalloc(sizeof(task_t));
    ready = 1;
}

task_t *get_current_task() {
    core_local_t *local = get_core_local(-1);
    return ddl_search(task_t, tasks, pid, local->pid);
}

int getpid() {
    task_t *task = get_current_task();
    return task->pid;
}

int getppid() {
    task_t *task = get_current_task();
    return task->ppid;
}

int setuid(int uid) {
    task_t *task = get_current_task();
    task->uid = uid;
    return 0;
}

int setppid(int ppid) {
    task_t *task = get_current_task();
    task->ppid = ppid;
    return 0;
}

int getpgrp(int pid) {
    task_t *task = ddl_search(task_t, tasks, pid, pid);
    if(task == NULL) {
        task = get_current_task();
    } 

    return task->pgrp;
}
