#include <mm/pmm.h>
#include <mm/slab.h>
#include <debug.h>
#include <mm/vmm.h>
#include <int/gdt.h>
#include <fs/vfs.h>
#include <acpi/rsdp.h>
#include <fs/fd.h>
#include <acpi/madt.h>
#include <drivers/hpet.h>
#include <int/apic.h>
#include <int/idt.h>
#include <sched/smp.h>
#include <fs/device.h>
#include <drivers/ahci.h>
#include <drivers/pci.h>
#include <sched/scheduler.h>

void ktask() {
    const char *argv[] = { "/test", NULL };
    const char *envp[] = { 
        "HOME=/",
        "PATH=/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin",
        "TERM=linux",
        NULL
    };

    sched_exec("/test", argv, envp, SCHED_USER | SCHED_ELF);

    for(;;)
        asm ("pause");
}

void kmain(void *stivale_phys) { 
    struct stivale *stivale = stivale_phys + HIGH_VMA;

    pmm_init(stivale_phys);
    cpu_init_features();
    vmm_init();

    rsdp_init((void*)(stivale->rsdp + HIGH_VMA));
    madt_init();
    init_hpet();

    gdt_init();
    apic_init();
    idt_init();

    devfs_init();
    pci_scan();

    vfs_mount_dev("/dev/SD0-0", "/");

    smp_init();

    tty_init(stivale);

    struct task *task = sched_create_task(NULL, NULL);
    sched_create_thread(task->pid, NULL, NULL, NULL, (uint64_t)ktask, 0x8);

    lapic_timer_init(100);

    asm ("sti");

    for(;;)
        asm ("pause");
}
