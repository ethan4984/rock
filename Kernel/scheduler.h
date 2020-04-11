#pragma once

#include <stdint.h>

void request_new_process(uint64_t range, void (*entry_function)());

void PITI();

void nanosleep(volatile uint32_t offset);

void sleep(volatile int offset);

void test_sleep();
