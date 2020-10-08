#pragma once

#include <kernel/mm/virtualPageManager.h>
#include <lib/asmUtils.h>

namespace sched {

void schedulerMain(regs_t *regs);

enum {
    WAITING,
    WAITING_TO_START,
    RUNNING
};

struct info {
    int status, idleCnt; 
    size_t entry; 
};

struct thread {
    int tid; 
    int pid; 
    info threadInfo;
    regs_t regs;

    void kill(); 
};
    
struct task {
    task() = default; 

    int pid;
    int ppid;
    int uid; 
    int threadCnt;
    info taskInfo;
    thread *threads;
    vmm::mapping pageMapping;
    regs_t regs;

    void exit();
    void createThread(uint16_t cs, size_t entry);
    
    static task getTask(int pid);
};

void createTask(uint16_t cs, size_t entry);

void init(); 

}
