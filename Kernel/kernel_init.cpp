#include <port.h>
#include <shitio.h>
#include <interrupt.h>
#include <keyboard.h>
#include <paging.h>
#include <process.h>
#include <stdint.h>
#include <graphics.h>

extern void load_gdt(void) asm("load_gdt");
extern void draw(void) asm("draw");

using namespace standardout;
using namespace MM;

extern "C" void kernel_main(void)
{
    load_gdt();
    initalize(VGA_WHITE, VGA_BLUE);
    t_print("\nKernel Debug");
    page_setup();
    idt_init();
    page_frame_init(0xF42400); //Reserves ~ 16mb

    asm volatile("sti");

    sprit_draw_main();
    clear_screen();

    /* tests processes allocation */

    process proc(0x2001);
    proc.pmalloc(0x4);
    uint16_t *ptrbruh = (uint16_t*)proc.pmalloc(0x8);
    proc.pfree(ptrbruh);
    uint16_t *ptruh = (uint16_t*)proc.pmalloc(0x4);

    block_show();
    draw_hline(VGA_MAGENTA, 6, 0, 66);
    k_print("\n\n\n");

    k_print("> ");
    startInput();

    for(;;);
}
