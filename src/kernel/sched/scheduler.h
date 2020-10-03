#pragma once

#include <lib/asmUtils.h>

#include <stdint.h>

extern "C" void startTask(uint64_t ss, uint64_t rsp, uint64_t cs, uint64_t entryPoint);

extern "C" void switchTask(uint64_t rsp, uint64_t dataSegment);

void schedulerMain(regs_t *regs);

inline uint64_t numberOfTasks = 0;
