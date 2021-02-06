#include <acpi/madt.h>
#include <asmutils.h>
#include <acpi/rsdp.h>
#include <int/apic.h>
#include <fs/fd.h>
#include <sched/scheduler.h>
#include <drivers/hpet.h>
#include <fs/vfs.h>
#include <drivers/ahci.h>
#include <drivers/pci.h>
#include <stivale.h>
#include <int/idt.h>
#include <sched/smp.h>
#include <fs/device.h>
#include <graphics.h>
#include <int/gdt.h>
#include <output.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <bitmap.h>

void kthread() {
    sched_exec("/test.elf", NULL, NULL, SCHED_USER | SCHED_ELF);
    for(;;)
        asm ("pause");
}

void kmain(void *stivale_phys) {
    stivale_t *stivale = stivale_phys + HIGH_VMA;

    pmm_init(stivale);

    bitmap_init();
    
    init_devfs();

    init_graphics(stivale);

    rsdp_init((rsdp_t*)(stivale->rsdp + HIGH_VMA));
    madt_init();

    gdt_init();
    idt_init();
    create_generic_tss();
    apic_init();

    pci_init();
    ahci_init();

    vfs_mount_dev("/dev/SD0-0", "/");

    init_hpet();

    init_smp();

    lapic_timer_init(50);

    vmm_init();
    
    task_t *ktask = sched_create_task(NULL, NULL);
    sched_create_thread(ktask->pid, (uint64_t)kthread, 0x8);

    asm ("sti");

    for(;;)
        asm ("pause");
}
