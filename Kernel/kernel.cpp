#include "port.h"
#include "shitio.h"
#include "interrupt.h"
#include "keyboard.h"
#include "paging.h"

extern void load_gdt(void) asm("load_gdt");

using namespace standardout;

extern "C" void kernel_main(void) {
	load_gdt();
	initalize();
	k_print("Starting crepOS\n");
	setup();
	idt_init();
	k_print("-------------------------------------------\n");

	outb(0x21, 0xFC);

	k_print("> ");
	startInput();

	while(1);
}
