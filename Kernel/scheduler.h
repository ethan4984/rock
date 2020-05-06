#pragma once

#include <stdint.h>

void request_new_process(uint64_t range, void (*entry_function)());

void nanosleep(volatile uint32_t offset);

void sleep(volatile int offset);

void test_sleep();

void init_scheduler();

extern "C" void start_process(uint64_t stack, uint64_t task) asm("start_process");
extern "C" void switch_process(uint64_t rsp, uint64_t rbp) asm("switch_process");

#define WAITING_TO_START 0
#define WAITING 1
#define RUNNING 2

class task
{
    public:
        uint8_t status;
        uint64_t rbp;
        uint64_t rsp;
        void *task;
};

void add_task(uint8_t *stack, void *main);
