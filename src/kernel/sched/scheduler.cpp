#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <kernel/sched/scheduler.h>
#include <kernel/sched/smp.h>
#include <kernel/int/apic.h>
#include <kernel/mm/kHeap.h>
#include <lib/output.h>

extern "C" void startTask(uint64_t ss, uint64_t rsp, uint64_t cs, uint64_t entryPoint);
extern "C" void switchTask(uint64_t rsp, uint64_t dataSegment);

namespace sched {

static task *tasks;
static int taskCnt = 0;

int debug = 0;

void schedulerMain(regs_t *regs) {
    static char lock = 0;
    spinLock(&lock);

    int nextTask = -1, lastTask = cpuInfo[regs->core].currentTask;
    for(int i = 0, cnt = 0; i < taskCnt; i++) { 
        if(tasks[i].taskInfo.status == WAITING && cnt < ++tasks[i].taskInfo.idleCnt) {
            cnt = tasks[i].taskInfo.idleCnt;
            nextTask = i;
        }

        if(tasks[i].taskInfo.status == WAITING_TO_START) {
            nextTask = i;
            break;
        }
    }

    if(nextTask == -1)
        goto end;

    if(lastTask != -1) {
        tasks[lastTask].regs = *regs; 
        tasks[lastTask].taskInfo.status = WAITING;
    }

    cpuInfo[regs->core]. currentTask = nextTask;

    tasks[nextTask].taskInfo.idleCnt = 0;
    tasks[nextTask].pageMapping.init();

    if(tasks[nextTask].taskInfo.status == WAITING_TO_START) {
        tasks[nextTask].taskInfo.status = RUNNING;
        apic::lapicWrite(LAPIC_EOI, 0); 
        spinRelease(&lock);
        startTask(tasks[nextTask].regs.ss, tasks[nextTask].regs.rsp, tasks[nextTask].regs.cs, tasks[nextTask].taskInfo.entry);
    }
    
    if(tasks[nextTask].taskInfo.status == WAITING) { 
        tasks[nextTask].taskInfo.status = RUNNING;
        apic::lapicWrite(LAPIC_EOI, 0);
        spinRelease(&lock); 
        switchTask((size_t)&tasks[nextTask].regs, tasks[nextTask].regs.ss);
    } else {
        kprintDS("[KDEBUG]", "kowalski analysis"); 
    }
end:
    spinRelease(&lock); 
    apic::lapicWrite(LAPIC_EOI, 0);
}

task task::getTask(int pid) {
    if(pid <= taskCnt)
        return tasks[pid];
    else {
        kprintDS("[KDEBUG]", "Error: requesting data for a non-existent processes");
        return (task) {};
    }
}

void createTask(uint16_t cs, size_t entry) {
    int index = taskCnt;
    
    task newTask;

    for(int i = 0; i < taskCnt; i++) {
        if(tasks[i].pid == -1) {
            newTask.pid = i;
            index = i; 
            break;
        }
    }

    newTask.pageMapping.setup();
    newTask.pageMapping.mapRange(0, 2, 0); 

    newTask.taskInfo.entry = entry;
    newTask.taskInfo.status = WAITING_TO_START;
    newTask.threads = new thread[10];

    newTask.regs.cs = cs;
    newTask.regs.ss = cs + 0x8;
    newTask.regs.rsp = pmm::alloc(2) + HIGH_VMA + 0x2000;

    if(++taskCnt % 10 == 0) {
        tasks = (task*)kheap.krealloc(tasks, 10);
    }
    
    tasks[index] = newTask;
}

void task::createThread(uint16_t cs, size_t entry) {
    thread newThread;

    newThread.pid = pid;
    newThread.regs.rsp = pmm::alloc(2) + HIGH_VMA;
    newThread.regs.cs = cs;
    newThread.regs.ss = cs + 0x8;
   
    int index = threadCnt;
    for(int i = 0; i < threadCnt; i++) {
        if(threads[i].tid == -1) {
            index = i;
            break; 
        }
    }

    if(++threadCnt % 10 == 0) {
        threads = (thread*)kheap.krealloc(threads, 10); 
    }

    threads[index] = newThread;
}

void task::exit() {
    pmm::free(regs.rsp - HIGH_VMA, 2);
    pid = -1;
}

void thread::kill() {
    pmm::free(regs.rsp - HIGH_VMA, 2);
    tid = -1;
}

void init() {
    tasks = new task[10];
}

}
