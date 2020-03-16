#include <port.h>
#include <shitio.h>
#include <interrupt.h>
#include <keyboard.h>
#include <paging.h>
#include <process.h>
#include <stdint.h>

extern void load_gdt(void) asm("load_gdt");

using namespace standardout;
using namespace MM;

extern "C" void kernel_main(void)
{
    load_gdt();
    initalize(VGA_BLUE, VGA_LIGHT_GREY);
    t_print("\nKernel Debug");
    k_print("Starting crepOS\n");
    page_setup();
    idt_init();
    page_frame_init(0xF42400); //Reserves ~ 16mb

    asm volatile("sti");

    /* tests processes allocation */

    process proc(0x2001);
    proc.pmalloc(0x4);
    uint16_t *ptrbruh = (uint16_t*)proc.pmalloc(0x8);
    proc.pfree(ptrbruh);
    uint16_t *ptruh = (uint16_t*)proc.pmalloc(0x4);

    start_counter(1, 0, 0x6);

    k_print("-------------------------------------------\n");

    k_print("> ");
    startInput();

    for(;;);
}
