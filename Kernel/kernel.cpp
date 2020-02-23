#include "port.h"
#include "shitio.h"
#include "interrupt.h"
#include "keyboard.h"

extern void load_gdt(void) asm("load_gdt");

using namespace standardout;

extern "C" void kernel_main(void) {
	load_gdt();
	initalize();
	k_print("Starting crepOS\n");
	idt_init();
	k_print("\n\n");
	outb(0x21, 0xFD);

	k_print("> ");
	startInput();


	while(1);
}
