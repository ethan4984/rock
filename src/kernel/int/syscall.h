#pragma once

#include <lib/asmUtils.h>

void syscallInit();

extern "C" void syscallMain(regs_t *regs);
