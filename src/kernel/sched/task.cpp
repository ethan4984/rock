#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <kernel/sched/scheduler.h>
#include <kernel/sched/task.h>
#include <kernel/mm/kHeap.h>
#include <lib/output.h>

uint64_t maxNumberOfTasks = 10;
static int64_t findFreeIndex();

void createTask(uint16_t ss, uint64_t rsp, uint16_t cs, uint64_t entryPoint, uint64_t pageCnt) {
    if(numberOfTasks % 10 == 0) {
        tasks = (task_t*)kheap.krealloc(tasks, sizeof(task_t) * 10);
        maxNumberOfTasks += 10;
    }

    int64_t index = findFreeIndex();

    if(index == -1) {
        cout + "[KDEBUG]" << "Extreme bruh momento\n";
        for(;;);
    }

    regs_t regs;

    regs.rsp = rsp;
    regs.ss = ss;
    regs.cs = cs; 

    task_t newTask = {  WAITING_TO_START, // status
                        0, //virtualPageManager.newUserMap(pageCnt), // pml4Index
                        regs,
                        pmm::alloc(2) + 0x2000 + HIGH_VMA, // kernelStack
                        entryPoint,
                        0,
                     };

    tasks[index] = newTask;
    numberOfTasks++;
}

static int64_t findFreeIndex() {
    for(uint64_t i = 0; i < maxNumberOfTasks; i++) {
        if(tasks[i].kernelStack == 0) 
            return i;
    }
    return -1;
}
