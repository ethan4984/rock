#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <kernel/drivers/ahci.h>
#include <kernel/drivers/pci.h>
#include <kernel/sched/hpet.h>
#include <kernel/acpi/madt.h>
#include <kernel/acpi/rsdp.h>
#include <kernel/mm/kHeap.h>
#include <kernel/int/apic.h>
#include <kernel/int/idt.h>
#include <kernel/int/gdt.h>
#include <kernel/int/tss.h>
#include <kernel/stivale.h>
#include <lib/output.h>

extern "C" void _init();

namespace kernel {

extern "C" void kernelMain(stivaleInfo_t *stivaleInfo) {
    _init(); /* calls all global constructors, dont put anything before here */

    physicalPageManager.init(stivaleInfo);
    kheap.init();
    virtualPageManager.init();

    idt.initIDT();
    idt.setIDTR();

    gdt.gdtInit();

    tssMain.init();
    tssMain.newTss(physicalPageManager.alloc(1) + HIGH_VMA); 

    gdt.initCore(0, (uint64_t)tssMain.tss);

    acpi.rsdpInit((uint64_t*)(stivaleInfo->rsdp + HIGH_VMA));

    madtInfo.madtInit();
    madtInfo.printMADT();
    initHPET(); 

    apic.initAPIC();

    asm volatile ("sti");

    pci.initPCI();
    ahci.initAHCI();

    for(;;);
}

}
