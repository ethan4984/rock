#include <shitio.h>
#include <multiboot.h>
#include <interrupt.h>
#include <graphics.h>
#include <shell.h>
#include <keyboard.h>
#include <paging.h>
#include <process.h>

using namespace standardout;
using namespace MM;

extern "C" void kernel_main(multiboot_info_t* info) {
    initalize(VGA_WHITE, VGA_BLUE);
    idt_init();

    asm volatile ("sti");

    start_counter(1, 0, 6);

    //sprit_draw_main();
    clear_screen();

    s_print(VGA_LIGHT_BLUE, 50, 1, "crepOS Dynamic Debugger");

    grab_current_y();
    draw_vline(VGA_MAGENTA, 48, 0, 25);

    page_frame_init(0xf42400);

    block_show();

    process proc(0x2001);
    proc.pmalloc(0x4);
    uint16_t *ptrbruh = (uint16_t*)proc.pmalloc(0x8);
    proc.pfree(ptrbruh);
    uint64_t *ptruh = (uint64_t*)proc.pmalloc(0x4);

    k_print("\n\n\n\n");

    k_print("> ");
    startInput();
    for(;;);
}
