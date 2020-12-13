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

static void reschedule(regs_t *regs) {
    core_local_t *local = get_core_local();

    int pid = -1, tid = -1;

    for(int i = 0, cnt = 0; i < max_task_cnt; i++) {
        if(tasks[i].exists != 1) 
            continue;

        if((tasks[i].status == WAITING) && (cnt < ++tasks[i].idle_cnt)) {
            cnt = tasks[i].idle_cnt;
            pid = i;
            continue;
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

    for(int i = 0, cnt = 0; i < next_task->max_thread_cnt; i++) {
        if(next_task->threads[i].exists != 1)
            continue;

        if((next_task->threads[i].status == WAITING) && (cnt < ++next_task->threads[i].idle_cnt)) {
            cnt = next_task->threads[i].idle_cnt;
            tid = i;
            continue; 
        }

        if(next_task->threads[i].status == WAITING_TO_START) {
            tid = i;
            break;
        }
    }

    if(tid == -1) {
        spin_release(&lock);
        return;
    }

    if((local->tid != -1) && (local->pid != -1)) {
        tasks[local->pid].threads[local->tid].regs = *regs;
        tasks[local->pid].threads[local->tid].status = WAITING;
        tasks[local->pid].status = WAITING;
    }

    local->pid = pid;
    local->tid = tid;

    thread_t *next_thread = &next_task->threads[tid];
    next_thread->idle_cnt = 0;

    switch(next_thread->status) {
        case WAITING:
            next_thread->status = RUNNING;

            lapic_write(LAPIC_EOI, 0); 
            spin_release(&lock);

            switch_task((uint64_t)&next_thread->regs, next_thread->regs.ss);
            break; 
        case WAITING_TO_START:
            next_thread->status = RUNNING;

            lapic_write(LAPIC_EOI, 0); 
            spin_release(&lock);

            kprintf("[KDEBUG]", "%x %x", next_thread->regs.ss, next_thread->regs.cs);

            start_task(next_thread->regs.ss, next_thread->regs.rsp, next_thread->regs.cs, next_thread->starting_addr);
            break;
        default:
            kprintf("[KDEBUG]", "Invalid thread %d/%d status = %d", pid, tid, next_thread->status);
    }
}

void scheduler_main(regs_t *regs) {
    spin_lock(&lock);

    if(!ready) { 
        spin_release(&lock);
        return;                 
    }

    reschedule(regs);

    lapic_write(LAPIC_EOI, 0); 
    spin_release(&lock);
}

void scheduler_init() {
    tasks = kcalloc(sizeof(task_t) * 0x200); 
    core_local = kmalloc(sizeof(core_local_t) * madt_info.ent0cnt);
    for(uint8_t i = 0; i < madt_info.ent0cnt; i++) {
        core_local[i] = (core_local_t) { -1, -1, i };
    }

    kvprintf("[SCHED] the scheduler is now pogging\n");
    ready = 1;
}

int create_task(uint64_t start_addr, uint16_t cs, uint16_t ss) {
    spin_lock(&lock);

    int pid = -1;
    for(int i = 0; i < max_task_cnt; i++) {
        if(tasks[i].exists == 0) {
            pid = i;
            break;
        }
    }

    if(pid == -1) {
        max_task_cnt += 0x200;
        tasks = krecalloc(tasks, sizeof(task_t) * max_task_cnt);
        spin_release(&lock);
        return create_task(start_addr, cs, ss);
    }

    tasks[pid] = (task_t) { .exists = 1,
                            .pid = pid,
                            .status = WAITING_TO_START,
                            .max_thread_cnt = 0x20,
                            .threads = kcalloc(sizeof(thread_t) * 0x20),
                            .cs = cs,
                            .ss = ss
                          };

    spin_release(&lock);

    create_task_thread(pid, start_addr);

    return pid;
}

int create_task_thread(int pid, uint64_t starting) {
    spin_lock(&lock);

    if(is_valid_pid(pid) == -1) {
        spin_release(&lock);
        return -1;
    }

    int tid = -1;
    for(int i = 0; i < tasks[pid].max_thread_cnt; i++) {
        if(tasks[pid].threads[i].exists == 0) {
            tid = i;
            break;
        }
    }

    if(tid == -1) {
        tasks[pid].max_thread_cnt += 0x20;
        tasks[pid].threads = krecalloc(tasks[pid].threads, sizeof(thread_t) * tasks[pid].max_thread_cnt);
        spin_release(&lock);
        return create_task_thread(pid, starting);
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

    tasks[pid].threads[tid].regs = (regs_t) {   .cs = tasks[pid].cs,
                                                .ss = tasks[pid].ss,
                                                .rsp = tasks[pid].threads[tid].kernel_stack
                                            };

    spin_release(&lock);

    return tid; 
}

int kill_task(int pid) {
    spin_lock(&lock);

    if(is_valid_pid(pid) == -1) {
        spin_release(&lock);
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

    spin_release(&lock);

    return 0;
}

int kill_thread(int pid, int tid) {
    spin_lock(&lock);

    if(is_valid_tid(pid, tid) == -1) {
        spin_release(&lock);
        return -1; 
    }

    thread_t *thread = &tasks[pid].threads[tid];

    thread->exists = 0;

    pmm_free(thread->kernel_stack, thread->ks_page_cnt);
    pmm_free(thread->user_stack, thread->us_page_cnt);

    spin_release(&lock);

    return 0;
}

task_t *get_current_task() {
    core_local_t *local = get_core_local();
    if(is_valid_pid(local->pid) == -1) {
        return NULL;
    }

    return &tasks[local->pid];
}
