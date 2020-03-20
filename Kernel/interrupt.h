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
extern uint64_t mouse() asm("mouse_handler");
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

extern "C" void panic(const char *message);

void reg_flow();
void reg_flow16();
void seg_flow();

void start_counter(int freq, uint8_t counter, uint8_t mode);
void sleep(int ticks);
