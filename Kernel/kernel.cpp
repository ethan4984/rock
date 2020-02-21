#include "port.h"
#include "shitio.h"
#include "interrupt.h"
#include "keyboard.h"


extern void load_gdt(void) asm("load_gdt");

extern "C" void kernel_main(void) {
	load_gdt();
	initalize();
	k_print("Starting crepOS\n");
	idt_init();
	outb(0x21, 0xFD);

	startInput();

	while(1);
}
