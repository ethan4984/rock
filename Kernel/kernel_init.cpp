#include <shitio.h>
#include <interrupt.h>
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
#include <stivale.h>
#include <vesa.h>
#include <mouse.h>

using namespace standardout;
using namespace MM;

extern void div_test() asm("test_div");
extern void _init() asm("_init");

extern "C" void kernel_main(stivale_info_t *boot_info)
{
    idt_init();

    asm volatile ("sti");

    start_counter(1, 0, 6);

    page_frame_init(120000000);

    blocks_init();

    _init();

    init_acpi();

    init_graphics(boot_info);

    uint32_t bruh[4][2] = { { 20, 20 }, { 20, 25 }, { 25, 20 }, { 25, 25} };

    vec2 desk = { { 0, 0 }, { 0, 1024 }, { 768, 0 }, { 1024, 768 } };

    window desktop(desk, 0xFFF, 0xFFF);

    sleep(1);

    widget thing(bruh, 4, 0xfffffff, 0xfffffff);

    sleep(1);

    thing.move_right(bruh, 4);
    thing.move_right(bruh, 4);
    thing.move_right(bruh, 4);
    thing.move_right(bruh, 4);

    sleep(1);

    thing.move_up(bruh, 4);
    thing.move_up(bruh, 4);
    thing.move_up(bruh, 4);
    thing.move_up(bruh, 4);

    sleep(1);

    thing.move_down(bruh, 4);
    thing.move_down(bruh, 4);
    thing.move_down(bruh, 4);
    thing.move_down(bruh, 4);
    thing.move_down(bruh, 4);
    thing.move_down(bruh, 4);
    thing.move_down(bruh, 4);
    thing.move_down(bruh, 4);
    thing.move_down(bruh, 4);

    mouse_setup();

    for(;;);
}
