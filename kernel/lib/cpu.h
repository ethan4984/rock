#ifndef CPU_H_
#define CPU_H_

#include <stddef.h>
#include <stdint.h>

#define MSR_EFER 0xc0000080
#define MSR_STAR 0xc0000081
#define MSR_LSTAR 0xc0000082
#define MSR_CSTAR 0xc0000083
#define MSR_SFMASK 0xc0000084

#define MSR_FS_BASE 0xc0000100
#define MSR_GS_BASE 0xc0000101
#define KERNEL_GS_BASE 0xc0000102

#define COM1 0x3f8 
#define COM2 0x2f8
#define COM3 0x3e8
#define COM4 0x2e8

struct regs {
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
} __attribute__((packed));

__attribute__((always_inline))
inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0,%1"::"a"(data), "Nd"(port));
}

inline void outw(uint16_t port, uint16_t data) {
    asm volatile("outw %0,%1"::"a"(data), "Nd"(port));
}

inline void outd(uint16_t port, uint32_t data) {
    asm volatile("outl %0, %1"::"a"(data), "Nd"(port));
}

inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile("inb %1, %0":"=a"(data):"Nd"(port));
    return data;
}

inline uint16_t inw(uint16_t port) {
    uint16_t data;
    asm volatile("inw %1, %0":"=a"(data):"Nd"(port));
    return data;
}

inline uint32_t ind(uint16_t port) {
    uint32_t data;
    asm volatile("inl %1, %0":"=a"(data):"Nd"(port));
    return data;
}

inline uint64_t rdmsr(uint64_t msr) {
    uint64_t rax, rdx;
    asm volatile ("rdmsr" : "=a"(rax), "=d"(rdx) : "c"(msr));
    return (rdx << 32) | rax;
}

inline void wrmsr(uint64_t msr, uint64_t data) {
    uint64_t rax = (uint32_t)data;
    uint64_t rdx = data >> 32;
    asm volatile ("wrmsr" :: "a"(rax), "d"(rdx), "c"(msr));
}

inline void swapgs(void) {
    asm volatile ("swapgs" ::: "memory");
}


static inline void set_kernel_gs(uintptr_t addr) {
    wrmsr(0xc0000101, addr);
}

static inline uint64_t get_kernel_gs() {
    return rdmsr(0xc0000101);
}

static inline void set_user_gs(uintptr_t addr) {
    wrmsr(0xc0000102, addr);
}

static inline void set_user_fs(uintptr_t addr) {
    wrmsr(0xc0000100, addr);
}

static inline uintptr_t get_user_gs(void) {
    return rdmsr(0xc0000102);
}

static inline uintptr_t get_user_fs(void) {
    return rdmsr(0xc0000100);
}

void cpu_init_features();
void set_errno(uint64_t errno);
uint64_t get_errno(uint64_t errno);

#endif
