#pragma once

#include <stdint.h>

struct IDT_entry {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t zero;
	uint8_t type_attr;
	uint16_t offset_high;
};

void idt_init(void);

extern void load_idt(unsigned long *idt_ptr) asm("load_idt"); /* non generic */
extern void keyboard_handler(void) asm("keyboard_handler");
extern void time_handler(void) asm("time_handler");

extern int irq2() asm("irq2");  /* generic: ToDo: Make not generic */
extern int irq3() asm("irq3");
extern int irq4() asm("irq4");
extern int irq5() asm("irq5");
extern int irq6() asm("irq6");
extern int irq7() asm("irq7");
extern int irq8() asm("irq8");
extern int irq9() asm("irq9");
extern int irq10() asm("irq10");
extern int irq11() asm("irq11");
extern int irq12() asm("irq12");
extern int irq13() asm("irq13");
extern int irq14() asm("irq14");
extern int irq15() asm("irq15");

void panic(const char *message, const char *proccess);

