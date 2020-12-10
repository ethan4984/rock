#include <sched/scheduler.h>
#include <fs/fd.h>
#include <acpi/madt.h>
#include <bitmap.h>
#include <output.h>
#include <sched/smp.h>

static int ready = 0, max_task_cnt = 0x200;
static task_t *tasks;
static core_local_t *core_local;
static char lock = 0;

core_local_t *get_core_local() {
    uint16_t core_index = 0;
    asm volatile ("mov %%gs, %0" : "=r"(core_index));
    return &core_local[core_index];
}

static void reschedule(regs_t *regs) {
    core_local_t *local = get_core_local();

    int pid = -1;
    int tid = -1;

    for(int i = 0, cnt = 0; i < max_task_cnt; i++) {
        if((tasks[i].exists == 1) && (tasks[i].status == WAITING) && (cnt < ++tasks[i].idle_cnt)) {
            cnt = tasks[i].idle_cnt;
            pid = i;
        }

        if(tasks[i].status == WAITING_TO_START) {
            pid = i;
            break;
        }
    }

    if(pid == -1) {
        return; 
    }

    task_t *next_task = &tasks[pid];
    next_task->idle_cnt = 0;

    for(int i = 0, cnt = 0; i < next_task->max_thread_cnt; i++) {
        if((next_task->exists == 1) && (next_task->threads[i].status == WAITING) && (cnt < ++next_task->threads[i].idle_cnt)) {
            cnt = next_task->threads[i].idle_cnt;
            tid = i;
        }

        if(next_task->threads[i].status == WAITING_TO_START) {
            tid = i;
            break;
        }
    }

    if(tid == -1) {
        return;
    }

    thread_t *next_thread = &next_task->threads[pid];
    next_thread->idle_cnt = 0;

    lapic_write(LAPIC_EOI, 0); 
    spin_release(&lock);

    switch(next_thread->status) {
        case WAITING:
            switch_task(next_thread->regs.rsp, next_thread->regs.ss);
            break;
        case WAITING_TO_START:
            start_task(next_thread->regs.ss, next_thread->regs.rsp, next_thread->regs.cs, next_thread->starting_addr);
            break;
        default:
            kprintf("[SCHED]", "Invalid thread %d/%d", pid, tid);
    }
}

static int is_valid_pid(int pid) {
    if((pid >= 0) || (pid <= (int)max_task_cnt)) {
        return 0;
    }
    return -1;
}

static int is_valid_tid(int pid, int tid) {
    if(is_valid_pid(pid) == 0) {
        if((tid >= 0) || (tid <= tasks[pid].max_thread_cnt)) {
            return 0;
        }
    }
    return -1;
}

void scheduler_main(regs_t *regs) {
    spin_lock(&lock);

    if(!ready) { 
        spin_release(&lock);
        return;                 
    }

    reschedule(regs);
}

void scheduler_init() {
    tasks = kcalloc(sizeof(task_t) * 0x200); 
    core_local = kmalloc(sizeof(core_local_t) * madt_info.ent0cnt);
    for(uint8_t i = 0; i < madt_info.ent0cnt; i++) {
        core_local[i] = (core_local_t) { -1, -1 };
    }

    kvprintf("[SCHED] the scheduler is now pogging\n");
    ready = 1;
}

int create_task(uint64_t start_addr) {
    int pid = -1;
    for(int i = 0; i < max_task_cnt; i++) {
        if(tasks[i].exists == 0) {
            pid = i;
        }
    }

    if(pid == -1) {
        max_task_cnt += 0x200;
        tasks = krecalloc(tasks, sizeof(task_t) * max_task_cnt);
        return create_task(start_addr);
    }

    tasks[pid] = (task_t) { .exists = 1,
                            .pid = pid,
                            .status = WAITING_TO_START,
                            .max_thread_cnt = 32,
                            .threads = kcalloc(sizeof(thread_t) * 32)
                          };

    return pid;
}

int create_task_thread(int pid, uint64_t starting) {
    if(is_valid_pid(pid) == -1) {
        return -1;
    }

    int tid = -1;
    for(int i = 0; i < max_task_cnt; i++) {
        if(tasks[i].exists == 0) {
            pid = i;
        }
    }

    if(tid == -1) {
        tasks[pid].max_thread_cnt += 0x1000;
        tasks[pid].threads = krecalloc(tasks[pid].threads, sizeof(thread_t) * tasks[pid].max_thread_cnt);
        return create_task(starting);
    }

    tasks[pid].threads[tid] = (thread_t) {  .exists = 1,
                                            .pid = pid,
                                            .status = WAITING_TO_START,
                                            .ks_page_cnt = 4,
                                            .us_page_cnt = 4,
                                            .kernel_stack = pmm_alloc(4),
                                            .user_stack = pmm_alloc(4),
                                            .starting_addr = starting
                                         };

    return tid; 
}

int kill_task(int pid) {
    if(is_valid_pid(pid) == -1) {
        return -1;
    }

    tasks[pid].exists = 0;

    for(int i = 0; i < tasks[pid].file_handle_cnt; i++) {
        close(tasks[pid].file_handles[i]);
    }

    for(int i = 0; i < tasks[pid].max_thread_cnt; i++) {
        if(tasks[pid].threads[i].exists == 1) {
            pmm_free(tasks[pid].threads[i].kernel_stack, tasks[pid].threads[i].ks_page_cnt);
            pmm_free(tasks[pid].threads[i].user_stack, tasks[pid].threads[i].us_page_cnt);
        }
    } 

    kfree(tasks[pid].threads);
    kfree(tasks[pid].file_handles);

    return 0;
}

int kill_thread(int pid, int tid) {
    if(is_valid_tid(pid, tid) == -1) {
        return -1; 
    }

    return 0;
}
