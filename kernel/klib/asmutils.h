#ifndef ASMUTILS_H_
#define ASMUTILS_H_

#include <stdint.h>

#define MSR_EFER 0xc0000080
#define MSR_STAR 0xc0000081
#define MSR_LSTAR 0xc0000082
#define MSR_CSTAR 0xc0000083
#define MSR_SFMASK 0xc0000084

#define MSR_FS_BASE 0xc0000100
#define MSR_GS_BASE 0xc0000101

typedef struct {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t isr_number;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs; 
    uint64_t rflags; 
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed)) regs_t;

void outb(uint16_t port, uint8_t data);

void outw(uint16_t port, uint16_t data);

void outd(uint16_t port, uint32_t data);

uint8_t inb(uint16_t port);

uint16_t inw(uint16_t port);

uint32_t ind(uint16_t port);

uint64_t rdmsr(uint64_t msr);

void wrmsr(uint64_t msr, uint64_t data);

#endif
