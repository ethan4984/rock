#include <sched/scheduler.h>
#include <sched/smp.h>
#include <cpu.h>

extern void outb(uint16_t port, uint8_t data);
extern void outw(uint16_t port, uint16_t data);
extern void outd(uint16_t port, uint32_t data);
extern uint8_t inb(uint16_t port);
extern uint16_t inw(uint16_t port);
extern uint32_t ind(uint16_t port);
extern uint64_t rdmsr(uint64_t msr);
extern void wrmsr(uint64_t msr, uint64_t data);
extern inline void swapgs(void);

extern void syscall_main();

void cpu_init_features() {
    wrmsr(MSR_EFER, rdmsr(MSR_EFER) | 1); // set SCE

    uint64_t cr0 = 0;
    asm volatile ( "mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 2);
    cr0 |= (1 << 1);
    asm volatile ( "mov %0, %%cr0" :: "r"(cr0));

    uint64_t cr4;
    asm volatile ( "mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 7) | (1 << 9) | (1 << 10);
    asm volatile ( "mov %0, %%cr4" :: "r"(cr4));

    wrmsr(MSR_STAR, 0x0013000800000000);
    wrmsr(MSR_LSTAR, (uint64_t)syscall_main);
    wrmsr(MSR_SFMASK, (uint64_t)~((uint32_t)0x002));
}

void syscall_set_fs_base(struct regs *regs) {
    struct core_local *local = get_core_local(CURRENT_CORE);
    struct task *current_task = hash_search(struct task, tasks, local->pid);
    struct thread *current_thread = hash_search(struct thread, current_task->threads, local->tid);

    set_user_fs(regs->rdi);    

    current_thread->user_fs_base = regs->rdi;
}

void syscall_set_gs_base(struct regs *regs) {
    struct core_local *local = get_core_local(CURRENT_CORE);
    struct task *current_task = hash_search(struct task, tasks, local->pid);
    struct thread *current_thread = hash_search(struct thread, current_task->threads, local->tid);

    set_user_gs(regs->rdi);

    current_thread->user_gs_base = regs->rdi;
}

void syscall_get_fs_base(struct regs *regs) {
    regs->rax = get_user_fs();
}

void syscall_get_gs_base(struct regs *regs) {
    regs->rax = get_user_gs();
}

void set_errno(uint64_t errno) {
    struct core_local *local = get_core_local(CURRENT_CORE);
    if(local == NULL) 
        return;
    local->errno = errno;
}

uint64_t get_errno(uint64_t errno) {
    struct core_local *local = get_core_local(CURRENT_CORE);
    if(local == NULL) 
        return 0;
    return local->errno;
}
