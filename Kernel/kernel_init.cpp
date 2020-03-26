#include <shitio.h>
#include <multiboot.h>
#include <interrupt.h>
#include <graphics.h>
#include <shell.h>
#include <keyboard.h>
#include <paging.h>
#include <process.h>
#include <scheduler.h>
#include <alloc.h>
#include <vector.h>
#include <string.h>

using namespace standardout;
using namespace MM;

void main() {
	t_print("Bruh");
}

void other_main() {
	t_print("no bruh");
}

extern void div_test() asm("test_div");

extern "C" void kernel_main()
{
    initalize(VGA_WHITE, VGA_BLUE);
    idt_init();

    asm volatile ("sti");

    start_counter(1, 0, 6);

    sprit_draw_main();
    clear_screen();

    s_print(VGA_LIGHT_BLUE, 50, 1, "crepOS 64 bit Dynamic Debugger");

    grab_current_y();
    draw_vline(VGA_MAGENTA, 48, 0, 25);

    page_frame_init(120000000);

    block_show();

    blocks_init();

    //div_test();

    k_print("\n\n\n\n");

    change_text_color(VGA_LIGHT_BLUE);
    k_print("(*ROOT*) > ");
    change_text_color(VGA_BLUE);
    startInput();
    for(;;);
}
