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
#include <sound.h>
#include <acpi.h>

using namespace standardout;
using namespace MM;

extern void div_test() asm("test_div");
extern void _init() asm("_init");

void drawing()
{
    sleep(2);
//    while(1);
/*    start_counter(1, 0, 6);

    clear_screen();

    sprit_draw_main();

    s_print(VGA_LIGHT_BLUE, 50, 1, "crepOS 64 bit Dynamic Debugger");

    grab_current_y();
    draw_vline(VGA_MAGENTA, 48, 0, 25); */
}

extern "C" void kernel_main()
{
    initalize(VGA_BLACK, VGA_WHITE);
    idt_init();

    asm volatile ("sti");

    start_counter(1, 0, 6);

    page_frame_init(120000000);

    block_show();

    t_print("Before");

    blocks_init();

    _init(); //needed for global constructors ~ be careful about putting things before this

    init_acpi();

    disable_cursor();

    mask_irq(1);

    for(int i = 4; i > 0; i--) {
        if(i == 0) {
            special_char('0', 1, 1, VGA_WHITE);
            sleep(1);
            continue;
        }
        special_num(i, count_digits(i), 2, 22, VGA_WHITE);
        sleep(1);
    }

    clear_irq(1);

    enable_cursor();

    clear_screen();
    initalize(VGA_WHITE, VGA_BLUE);

    change_text_color(VGA_LIGHT_BLUE);
    k_print("(*ROOT*) > ");
    change_text_color(VGA_BLUE);
    startInput();

    for(;;);
}
