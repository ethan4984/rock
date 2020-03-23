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

using namespace standardout;
using namespace MM;


/* test vector class */
void br()
{
    vector<int> v;
    t_print("size: %d", v.size());
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);
    v.push_back(40);
    v.push_back(50);

    v.erase(1);

    t_print("size: %d", v.size());


    for(int i = 0; i < v.used(); i++)
        t_print("%d ", v.at(i));
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

    br();

    //div_test(); /* tests exceptions */

    /*process proc(0x2001, main);

    proc.pmalloc(0x4);
    uint16_t *ptrbruh = (uint16_t*)proc.pmalloc(0x8);
    proc.pfree(ptrbruh);
    uint64_t *ptruh = (uint64_t*)proc.pmalloc(0x4);*/

    k_print("\n\n\n\n");

    change_text_color(VGA_LIGHT_BLUE);
    k_print("(*ROOT*) > ");
    change_text_color(VGA_BLUE);
    startInput();
    for(;;);
}
