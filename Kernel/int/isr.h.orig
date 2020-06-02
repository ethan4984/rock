#pragma once

#include <stdint.h>

void panic(const char *str, ...);

typedef struct registers_t {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rsi, rdi, rbp, rdx, rcx, rbx, rax, exceptionIndex;
} registers_t;

const char *isrMessages[] = {   "Divide-by-zero Error", "Debug", "NMI", "Breakpoint", "Overflow", "Bound Range Exceeded",
                                "Invaild Opcode", "Device Not Available", "Double Fault", "Coprocessor Segment Overrun",
                                "Invaild TSS", "Segment Not Present", "Stack-Segment Fault", "General Protecteion Fault",
                                "Page Fault", "x87 Floating-Point", "Alignment Check", "Machine Check", "SIMD Floating-Point",
                                "Virtualization Excpetion"
                            };
