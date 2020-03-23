#pragma once

#include <stdint.h>
#include <stddef.h>

struct IDT_entry
{
    uint16_t low_offset;
    uint16_t selector;
    uint8_t zero8;
    uint8_t type_addr;
    uint16_t middle_offset;
    uint32_t high_offset;
    uint32_t zero32;
} __attribute__((packed));

typedef struct
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idtr;

typedef struct
{
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;

    uint64_t rdi;
    uint64_t rsi;
    uint64_t rbp;
    uint64_t rsp;

    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;

    uint64_t rip;
} __attribute__((packed)) registers;

typedef struct
{
    uint16_t ss;
    uint16_t cs;
    uint16_t ds;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;
} __attribute__((packed)) seg_register;

void idt_init(void);
void mask_irq(unsigned char channel);
void clear_irq(unsigned char channel);

extern void load_idt(unsigned long *idt_ptr) asm("load_idt"); /* non generic */
extern void keyboard_handler(void) asm("keyboard_handler");
extern void time_handler(void) asm("time_handler");

extern uint64_t irq2() asm("irq2");  /* generic: ToDo: Make not generic */
extern uint64_t irq3() asm("irq3");
extern uint64_t irq4() asm("irq4");
extern uint64_t irq5() asm("irq5");
extern uint64_t irq6() asm("irq6");
extern uint64_t irq7() asm("irq7");
extern uint64_t irq8() asm("irq8");
extern uint64_t irq9() asm("irq9");
extern uint64_t irq10() asm("irq10");
extern uint64_t irq11() asm("irq11");
extern uint64_t mouse() asm("irq12");
extern uint64_t irq13() asm("irq13");
extern uint64_t irq14() asm("irq14");
extern uint64_t irq15() asm("irq15");

extern void divide_zero(void) asm("divide_by_zero");
extern void debug(void) asm("debug");
extern void non_maskable_irq(void) asm("non_maskable_irq");
extern void breakpoint(void) asm("breakpoint");
extern void overflow(void) asm("overflow");
extern void bound_range(void) asm("bound_range");
extern void invaild_opcode(void) asm("invaild_opcode");
extern void device_not_available(void) asm("device_not_available");
extern void double_fault(void) asm("double_fault");
extern void coprocessor_seg_overrun(void) asm("coprocessor_seg_overrun");
extern void invaild_tss(void) asm("invaild_tss");
extern void segment_not_found(void) asm("segment_not_found");
extern void stack_seg_fault(void) asm("stack_seg_fault");
extern void gen_fault(void) asm("gen_fault");
extern void page_fault(void) asm("page_fault");
extern void floating_point_fault(void) asm("floating_point_fault");
extern void alignment_check(void) asm("alignment_check");
extern void machine_check(void) asm("machine_check");
extern void simd_floating_point(void) asm("simd_floating_point");
extern void vm_expection(void) asm("vm_expection");
extern void security_expection(void) asm("security_expection");
extern void reserved(void) asm("reserved");

extern "C" void panic(const char *message);

void reg_flow();
void seg_flow();

void start_counter(int freq, uint8_t counter, uint8_t mode);
void sleep(int ticks);
