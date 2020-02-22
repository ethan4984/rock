#include "interrupt.h"
#include "port.h"
#include "shitio.h"

struct IDT_entry IDT[256];

extern void load_idt(unsigned long *idt_ptr)  asm("load_idt");
extern void keyboard_handler(void) asm("keyboard_handler"); /* non-generic */
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

void idt_gate(uint32_t referenceIRQ) {

	static int counter = 32;

	uint32_t irqX = referenceIRQ;
	IDT[counter].offset_lowerbits = irqX & 0xffff;
	IDT[counter].selector = 0x08;
	IDT[counter].zero = 0;
	IDT[counter].type_attr = 0x8e;
	IDT[counter].offset_higherbits = (irqX & 0xffff0000) >> 16;

	counter++;
}

extern "C" void irq_l(void) {
	outb(0x20, 0x20);
}

extern "C" void irq_h(void) {
	outb(0xA0, 0x20);
	outb(0x20, 0x20);
}

extern "C" void PITI() {
	outb(0x20, 0x20);
}

void idt_init(void) {
	uint32_t idt_address;
	uint32_t idt_ptr[2];

	outb(0x20, 0x11);
        outb(0xA0, 0x11);
        outb(0x21, 0x20);
        outb(0xA1, 40);
        outb(0x21, 0x04);
        outb(0xA1, 0x02);
        outb(0x21, 0x01);
        outb(0xA1, 0x01);
        outb(0x21, 0x0);
        outb(0xA1, 0x0);

	idt_gate((uint32_t)time_handler);
	idt_gate((uint32_t)keyboard_handler);
	idt_gate((uint32_t)irq2);
	idt_gate((uint32_t)irq3);
	idt_gate((uint32_t)irq4);
	idt_gate((uint32_t)irq5);
	idt_gate((uint32_t)irq6);
	idt_gate((uint32_t)irq7);
	idt_gate((uint32_t)irq8);
	idt_gate((uint32_t)irq9);
	idt_gate((uint32_t)irq10);
	idt_gate((uint32_t)irq11);
	idt_gate((uint32_t)irq12);
	idt_gate((uint32_t)irq13);
	idt_gate((uint32_t)irq14);
	idt_gate((uint32_t)irq15);

	idt_address = (uint32_t)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * 256) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	load_idt(idt_ptr);
}
