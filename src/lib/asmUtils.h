#pragma once

#define COM1 0x3f8 
#define COM2 0x2f8
#define COM3 0x3e8
#define COM4 0x2e8

#include <stdint.h>

typedef void *symbol[];

struct [[gnu::packed]] regs_t {
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
    uint64_t core;
    uint64_t isrNumber;
    uint64_t errorCode;
    uint64_t rip;
    uint64_t cs; 
    uint64_t rflags; 
    uint64_t rsp;
    uint64_t ss; 
};

void outb(uint16_t port, uint8_t data);

void outw(uint16_t port, uint16_t data);

void outd(uint16_t port, uint32_t data);

uint8_t inb(uint16_t port);

uint16_t inw(uint16_t port);

uint32_t ind(uint16_t port);

uint64_t rdmsr(uint64_t msr);

void wrmsr(uint64_t msr, uint64_t data);

uint8_t serialRead();

void serialWrite(uint8_t data);

void serialWriteString(const char *str);
