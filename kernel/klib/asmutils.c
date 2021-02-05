#include <asmutils.h>

void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0,%1"::"a"(data), "Nd"(port));
}

void outw(uint16_t port, uint16_t data) {
    asm volatile("outw %0,%1"::"a"(data), "Nd"(port));
}

void outd(uint16_t port, uint32_t data) {
    asm volatile("outl %0, %1"::"a"(data), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile("inb %1, %0":"=a"(data):"Nd"(port));
    return data;
}

uint16_t inw(uint16_t port) {
    uint16_t data;
    asm volatile("inw %1, %0":"=a"(data):"Nd"(port));
    return data;
}

uint32_t ind(uint16_t port) {
    uint32_t data;
    asm volatile("inl %1, %0":"=a"(data):"Nd"(port));
    return data;
}

uint64_t rdmsr(uint64_t msr) {
    uint64_t rax, rdx;
    asm volatile ("rdmsr" : "=a"(rax), "=d"(rdx) : "c"(msr));
    return (rdx << 32) | rax;
}

void wrmsr(uint64_t msr, uint64_t data) {
    uint64_t rax = (uint32_t)data;
    uint64_t rdx = data >> 32;
    asm volatile ("wrmsr" :: "a"(rax), "d"(rdx), "c"(msr));
}

void syscall_set_fs_base(regs_t *regs) {
    set_user_fs(regs->rdi);    
}

void syscall_set_gs_base(regs_t *regs) {
    set_user_gs(regs->rdi);
}

void syscall_get_fs_base(regs_t *regs) {
    regs->rax = get_user_fs();
}

void syscall_get_gs_base(regs_t *regs) {
    regs->rax = get_user_gs();
}
