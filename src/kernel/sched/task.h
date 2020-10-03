#pragma once

#include <lib/asmUtils.h>

enum {
    WAITING_TO_START = 1, 
    RUNNING,
    WAITING,
};

struct task_t {
    uint8_t status;
    uint64_t pml4Index;
    regs_t regs;
    uint64_t kernelStack;
    uint64_t entryPoint;
    uint64_t idleTime;
};

inline task_t *tasks;

void createTask(uint16_t ss, uint64_t rsp, uint16_t cs, uint64_t entryPoint, uint64_t pageCnt);
