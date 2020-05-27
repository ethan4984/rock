#pragma once

#include <stdint.h>
#include <Kernel/sched/task.h>

enum {
    WAITING2START,
    WAITING,
    RUNNING,
    DEAD
};

void startCounter(int frequency, uint8_t counter, uint8_t mode);

void sleep(volatile uint64_t waitTime);

void initScheduler();

void createThread(void *main, uint64_t stackSizeOverride);

void initTask(task_t newTask);

void killTask(uint64_t index);
