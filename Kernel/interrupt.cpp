#include "interrupt.h"
#include "port.h"
#include "shitio.h"
#include "keyboard.h"

extern void load_idt(unsigned long *idt_ptr)  asm("load_idt");
extern void keyboard_handler(void) asm("keyboard_handler");
extern unsigned char keyboard_map[128];

struct IDT_entry IDT[256];

void idt_init(void)
{
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

	uint32_t irq1; /* only supports irq1 interrupt for keyboard atm, more will be added later */

	irq1 = (uint32_t)keyboard_handler;
	IDT[33].offset_lowerbits = irq1 & 0xffff;
	IDT[33].selector = 0x08;
	IDT[33].zero = 0;
	IDT[33].type_attr = 0x8e;
	IDT[33].offset_higherbits = (irq1 & 0xffff0000) >> 16;

	idt_address = (uint32_t)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * 256) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	load_idt(idt_ptr);
}

extern "C" void keyboard_handler_main(void) {
	outb(0x20, 0x20);
	if(inb(0x64) & 0x01) {
		char keycode = inb(0x60);
		if(keycode < 0)
			return;
		switch(keycode) {
			case 0x1c:
				putchar('\n');
			case 0x0e:
				putchar('\b');
			default:
				putchar(keyboard_map[(unsigned char) keycode]);
		}
	}
}
