#include <kernel/mm/virtualPageManager.h>
#include <kernel/sched/scheduler.h>
#include <kernel/sched/task.h>
#include <kernel/sched/smp.h>
#include <kernel/int/apic.h>
#include <kernel/mm/kHeap.h>
#include <lib/output.h>

namespace kernel {

void schedulerMain(regs_t *regs) {
    static char lock = 0;
    spinLock(&lock);

    int64_t nextTask = -1, lastTask = cpuInfo[regs->core].currentTask;
    uint64_t cnt = 0;

    if(numberOfTasks == 0)
        goto end;

    for(uint64_t i = 0; i < numberOfTasks; i++) { /* find the next task to run */
        if(tasks[i].status == WAITING) {
            if(cnt < ++tasks[i].idleTime) {
                cnt = tasks[i].idleTime;
                nextTask = i; 
            }
        }

        if(tasks[i].status == WAITING_TO_START) {
            nextTask = i; 
            break;
        }
    }

    if(nextTask == -1) 
        goto end;

    if(lastTask != -1) {
        tasks[lastTask].regs = *regs;
        tasks[lastTask].status = WAITING;
    }

    cpuInfo[regs->core].currentTask = nextTask;

    tasks[nextTask].idleTime = 0;
//    virtualPageManager.initAddressSpace(tasks[nextTask].pml4Index); // set pml4 as whatever its supposed tp be

    if(tasks[nextTask].status == WAITING_TO_START) {
        tasks[nextTask].status = RUNNING;
        spinRelease(&lock);
        apic.lapicWrite(LAPIC_EOI, 0);
        startTask(tasks[nextTask].regs.ss, tasks[nextTask].regs.rsp, tasks[nextTask].regs.cs, tasks[nextTask].entryPoint);
    }

    if(tasks[nextTask].status == WAITING) {
        tasks[nextTask].status = RUNNING;
        spinRelease(&lock);
        apic.lapicWrite(LAPIC_EOI, 0);
        switchTask((uint64_t)&tasks[nextTask].regs, tasks[nextTask].regs.ss);
    }
end:
    spinRelease(&lock);
    return;
}

}
