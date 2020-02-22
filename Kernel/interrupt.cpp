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

	k_print("Filling idt");

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

	uint32_t irq0;
	uint32_t irq1;
	uint32_t irq2_address;
	uint32_t irq3_address;
	uint32_t irq4_address;
	uint32_t irq5_address;
	uint32_t irq6_address;
	uint32_t irq7_address;
	uint32_t irq8_address;
	uint32_t irq9_address;
	uint32_t irq10_address;
	uint32_t irq11_address;
	uint32_t irq12_address;
	uint32_t irq13_address;
	uint32_t irq14_address;
	uint32_t irq15_address;

	irq0 = (uint32_t)time_handler;
	IDT[32].offset_lowerbits = irq0 & 0xffff;
	IDT[32].selector = 0x08;
	IDT[32].zero = 0;
	IDT[32].type_attr = 0x8e;
	IDT[32].offset_higherbits = (irq0 & 0xffff0000) >> 16;

	irq1 = (uint32_t)keyboard_handler;
	IDT[33].offset_lowerbits = irq1 & 0xffff;
	IDT[33].selector = 0x08;
	IDT[33].zero = 0;
	IDT[33].type_attr = 0x8e;
	IDT[33].offset_higherbits = (irq1 & 0xffff0000) >> 16;

	irq2_address = (uint32_t)irq2;
	IDT[34].offset_lowerbits = irq2_address & 0xffff;
	IDT[34].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[34].zero = 0;
	IDT[34].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[34].offset_higherbits = (irq2_address & 0xffff0000) >> 16;

	irq3_address = (uint32_t)irq3;
	IDT[35].offset_lowerbits = irq3_address & 0xffff;
	IDT[35].selector = 0x08;
	IDT[35].zero = 0;
	IDT[35].type_attr = 0x8e;
	IDT[35].offset_higherbits = (irq3_address & 0xffff0000) >> 16;

	irq4_address = (uint32_t)irq4;
	IDT[36].offset_lowerbits = irq4_address & 0xffff;
	IDT[36].selector = 0x08;
	IDT[36].zero = 0;
	IDT[36].type_attr = 0x8e;
	IDT[36].offset_higherbits = (irq4_address & 0xffff0000) >> 16;

	irq5_address = (uint32_t)irq5;
	IDT[37].offset_lowerbits = irq5_address & 0xffff;
	IDT[37].selector = 0x08;
	IDT[37].zero = 0;
	IDT[37].type_attr = 0x8e;
	IDT[37].offset_higherbits = (irq5_address & 0xffff0000) >> 16;

	irq6_address = (uint32_t)irq6;
	IDT[38].offset_lowerbits = irq6_address & 0xffff;
	IDT[38].selector = 0x08;
	IDT[38].zero = 0;
	IDT[38].type_attr = 0x8e;
	IDT[38].offset_higherbits = (irq6_address & 0xffff0000) >> 16;

	irq7_address = (uint32_t)irq7;
	IDT[39].offset_lowerbits = irq7_address & 0xffff;
	IDT[39].selector = 0x08;
	IDT[39].zero = 0;
	IDT[39].type_attr = 0x8e;
	IDT[39].offset_higherbits = (irq7_address & 0xffff0000) >> 16;

	irq8_address = (uint32_t)irq8;
	IDT[40].offset_lowerbits = irq8_address & 0xffff;
	IDT[40].selector = 0x08;
	IDT[40].zero = 0;
	IDT[40].type_attr = 0x8e;
	IDT[40].offset_higherbits = (irq8_address & 0xffff0000) >> 16;

	irq9_address = (uint32_t)irq9;
	IDT[41].offset_lowerbits = irq9_address & 0xffff;
	IDT[41].selector = 0x08;
	IDT[41].zero = 0;
	IDT[41].type_attr = 0x8e;
	IDT[41].offset_higherbits = (irq9_address & 0xffff0000) >> 16;

	irq10_address = (uint32_t)irq10;
	IDT[42].offset_lowerbits = irq10_address & 0xffff;
	IDT[42].selector = 0x08;
	IDT[42].zero = 0;
	IDT[42].type_attr = 0x8e;
	IDT[42].offset_higherbits = (irq10_address & 0xffff0000) >> 16;

	irq11_address = (uint32_t)irq11;
	IDT[43].offset_lowerbits = irq11_address & 0xffff;
	IDT[43].selector = 0x08;
	IDT[43].zero = 0;
	IDT[43].type_attr = 0x8e;
	IDT[43].offset_higherbits = (irq11_address & 0xffff0000) >> 16;

	irq12_address = (uint32_t)irq12;
	IDT[44].offset_lowerbits = irq12_address & 0xffff;
	IDT[44].selector = 0x08;
	IDT[44].zero = 0;
	IDT[44].type_attr = 0x8e;
	IDT[44].offset_higherbits = (irq12_address & 0xffff0000) >> 16;

	irq13_address = (uint32_t)irq13;
	IDT[45].offset_lowerbits = irq13_address & 0xffff;
	IDT[45].selector = 0x08;
	IDT[45].zero = 0;
	IDT[45].type_attr = 0x8e;
	IDT[45].offset_higherbits = (irq13_address & 0xffff0000) >> 16;

	irq14_address = (uint32_t)irq14;
	IDT[46].offset_lowerbits = irq14_address & 0xffff;
	IDT[46].selector = 0x08;
	IDT[46].zero = 0;
	IDT[46].type_attr = 0x8e;
	IDT[46].offset_higherbits = (irq14_address & 0xffff0000) >> 16;

        irq15_address = (uint32_t)irq15;
	IDT[47].offset_lowerbits = irq15_address & 0xffff;
	IDT[47].selector = 0x08;
	IDT[47].zero = 0;
	IDT[47].type_attr = 0x8e;
	IDT[47].offset_higherbits = (irq15_address & 0xffff0000) >> 16;

	idt_address = (uint32_t)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * 256) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	k_print(": idt adress: %x\n", idt_address);

	load_idt(idt_ptr);
}
