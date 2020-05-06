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
#include <pci.h>
#include <ahci.h>

extern pci_device_t *pci_devices;
extern pci_device_id_t *pci_device_ids;
extern uint64_t total_devices;

using namespace standardout;
using namespace MM;

extern void div_test() asm("test_div");
extern void _init() asm("_init");

uint32_t bruh_w[4][2] = { { 512, 512 }, { 512, 532 }, { 532, 512 }, { 532, 532 } };

void kernel_task(void)
{
    mouse_setup();

    for(;;);
}

extern "C" void kernel_main(stivale_info_t *boot_info)
{
    init_graphics(boot_info);

    vec2 desk = { { 0, 0 }, { 0, 1024 }, { 768, 0 }, { 1024, 768 } };

    window desktop(desk, 0xFFF, 0xFFF);

    initalize(0xffffffff, 0xfff, 1024, 786);

    show_vesa_state();

    page_frame_init(120000000);

    block_show();

    blocks_init();

    _init(); // needed for global constructors

    init_acpi();

    pci_init();

    idt_init();

    asm volatile ("sti");

    start_counter(1000, 0, 6);

    ahci_init(pci_devices, pci_device_ids, total_devices);

    init_scheduler();

    create_task((void*)kernel_task);

    for(;;);
}
