#include <mm/vmm.hpp>
#include <mm/pmm.hpp>
#include <mm/slab.hpp>

#include <int/idt.hpp>
#include <int/gdt.hpp>
#include <int/apic.hpp>

#include <fs/dev.hpp>
#include <fs/vfs.hpp>
#include <fs/fd.hpp>

#include <drivers/hpet.hpp>
#include <drivers/tty.hpp>
#include <drivers/pci.hpp>

#include <sched/smp.hpp>
#include <sched/scheduler.hpp>

#include <debug.hpp>
#include <stivale.hpp>
#include <font.hpp>

static stivale *stivale_virt = NULL;

extern "C" void _init();
extern "C" void __cxa_pure_virtual() { for(;;); }

extern "C" int main(size_t stivale_phys) {
    cpuid_state cpu_id = cpuid(7, 0);

    if(cpu_id.rcx & (1 << 16)) {
        vmm::high_vma = 0xff00000000000000;
    }

    stivale_virt = reinterpret_cast<stivale*>(stivale_phys + vmm::high_vma);

    pmm::init(stivale_virt);

    kmm::cache(NULL, 0, 32);
    kmm::cache(NULL, 0, 64);
    kmm::cache(NULL, 0, 128);
    kmm::cache(NULL, 0, 256);
    kmm::cache(NULL, 0, 512);
    kmm::cache(NULL, 0, 1024);
    kmm::cache(NULL, 0, 2048);
    kmm::cache(NULL, 0, 4096);
    kmm::cache(NULL, 0, 8192);
    kmm::cache(NULL, 0, 16384);
    kmm::cache(NULL, 0, 32768);
    kmm::cache(NULL, 0, 65536);
    kmm::cache(NULL, 0, 131072);
    kmm::cache(NULL, 0, 262144);

    vmm::init();

    _init();

    x86::gdt_init();
    x86::idt_init();
    
    new x86::tss;

    acpi::rsdp_ptr = (acpi::rsdp*)(stivale_virt->rsdp + vmm::high_vma);

    if(acpi::rsdp_ptr->xsdt_addr) { 
        acpi::xsdt_ptr = (acpi::xsdt*)(acpi::rsdp_ptr->xsdt_addr + vmm::high_vma);
        print("[ACPI] xsdt found at {x}\n", (size_t)(acpi::xsdt_ptr));
    } else {
        acpi::rsdt_ptr = (acpi::rsdt*)(acpi::rsdp_ptr->rsdt_addr + vmm::high_vma);
        print("[ACPI] rsdt found at {x}\n", (size_t)(acpi::rsdt_ptr));
    }

    lib::vector<vmm::region> region_list;

    cpu_init_features();
    init_hpet();

    apic::init();
    smp::boot_aps();

    dev::init();

    tty::screen screen(stivale_virt);
    new tty::tty(screen, (uint8_t*)font_bitmap, 16, 8);

    asm ("sti");

    pci::init();

    apic::timer_calibrate(100);

    vfs::mount("sd0-0", "/");

    const char *argv[] = { "/usr/bin/bash", NULL };
    const char *envp[] = {
        NULL
    };

    sched::arguments args(argv, envp); 

    sched::task *new_task = new sched::task(-1);
    new_task->exec("/usr/bin/bash", 0x23, args);

    for(;;)
        asm ("pause");
}
