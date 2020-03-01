#include "port.h"
#include "shitio.h"
#include "interrupt.h"
#include "keyboard.h"
#include "paging.h"

extern void load_gdt(void) asm("load_gdt");

using namespace standardout;
using namespace MM;

extern "C" void kernel_main(void) {
    load_gdt();
    initalize(VGA_BLUE, VGA_LIGHT_GREY);
    k_print("Starting crepOS\n");
    setup();
    idt_init();
    page_frame_init(0xF42400); //Reserves ~ 16mb
    k_print("-------------------------------------------\n");

    uint32_t *ptr = (uint32_t*)malloc(sizeof(uint32_t));
    uint16_t *ptr1 = (uint16_t*)malloc(sizeof(uint16_t));
    uint16_t *ptr2 = (uint16_t*)malloc(sizeof(uint16_t));
	k_print("Lord plz: %x\n", ptr);
	k_print("Lord plz: %x\n", ptr1);
	k_print("Lord plz: %x\n", ptr2);
	free(ptr2);
	uint16_t *ptr3 = (uint16_t*)malloc(sizeof(uint16_t));
	k_print("Really Lord plz: %x\n", ptr3);
	uint16_t *ptr4 = (uint16_t*)malloc(sizeof(uint16_t));
	k_print("Thank god: %x\n", ptr4);

    k_print("> ");
    startInput();

    for(;;);
}
