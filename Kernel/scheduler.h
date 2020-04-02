#pragma once

void request_new_process(uint64_t range, void (*entry_function)());

void PITI();

void nanosleep(volatile int offset);
