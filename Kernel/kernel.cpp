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

    malloc(sizeof(uint32_t));

    k_print("> ");
    startInput();

    for(;;);
}
