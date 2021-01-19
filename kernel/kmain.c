#include <acpi/madt.h>
#include <acpi/rsdp.h>
#include <int/apic.h>
#include <sched/scheduler.h>
#include <fs/fd.h>
#include <drivers/hpet.h>
#include <drivers/ahci.h>
#include <drivers/pci.h>
#include <stivale.h>
#include <int/idt.h>
#include <sched/smp.h>
#include <graphics.h>
#include <int/gdt.h>
#include <bmp.h>
#include <output.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <bitmap.h>

#define COL 0xff

void kmain(stivale_t *stivale) {
    pmm_init(stivale);

    bitmap_init();

    vmm_init();

    init_graphics(stivale);

    rsdp_init((rsdp_t*)stivale->rsdp);
    madt_init();

    gdt_init();
    idt_init();
    create_generic_tss();
    apic_init();

    vfs_init();

    pci_init();
    ahci_init();

    init_fd();

    partition_mount_all();

    init_hpet(); 

    init_smp();

    scheduler_init();

    lapic_timer_init(50);

    background_image_t bmp;
    bmp_draw("/background.bmp", &bmp, 0, 0);
    kvprintf("[LMAO]");

    uint32_t colour_buffer[] = {    COL, COL, COL, COL, COL, COL, COL,
                                    COL, COL, COL, COL, COL, COL, COL,
                                    COL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, COL,
                                    COL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, COL,
                                    COL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, COL,
                                    COL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, COL,
                                    COL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, COL,
                                    COL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, COL,
                                    COL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, UNUSED_PIXEL, COL,
                                    COL, COL, COL, COL, COL, COL, COL,
                                    COL, COL, COL, COL, COL, COL, COL
                                };

    shape_t shape = { .x = 100, .y = 100, .height = 11, .width = 7, .colour_buffer = colour_buffer, .backbuffer = kmalloc(sizeof(colour_buffer)) };

    draw_shape(&shape); 

    ksleep(1000);

    redraw_shape(&shape, 300, 300);

    asm ("sti");

    for(;;)
        asm ("pause");
}
