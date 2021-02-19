#include <sched/smp.h>
#include <int/idt.h>
#include <int/gdt.h>
#include <int/apic.h>
#include <debug.h>
#include <acpi/madt.h>
#include <drivers/hpet.h>

extern symbol smp_core_init_begin;
extern symbol smp_core_init_end;

static_vec(struct core_local, core_locals);

static void prep_core_init(size_t stack, uint64_t pml4, uint64_t entry, uint64_t idt, uint64_t gdt, uint64_t core_index) { 
    uint64_t *parms = (uint64_t*)(0x500 + HIGH_VMA);
    parms[0] = stack;
    parms[1] = pml4;
    parms[2] = entry;
    parms[3] = idt;
    parms[4] = gdt;
    parms[5] = core_index;
}

static void bootstrap_core(size_t core_index) {
    wrmsr(MSR_GS_BASE, (size_t)vec_search(struct core_local, core_locals, core_index));
    cpu_init_features();
    create_generic_tss();
    lapic_write(LAPIC_SINT, lapic_read(LAPIC_SINT) | 0x1ff);

    kprintf("Hi from core %d\n", core_index);

    for(;;);
}

void smp_init() {
    memcpy8((uint8_t*)(0x1000 + HIGH_VMA), (uint8_t*)smp_core_init_begin, (uint64_t)smp_core_init_end - (uint64_t)smp_core_init_begin);

    struct idtr idtr;
    asm volatile ("sidt %0" :: "m"(idtr));

    struct gdtr gdtr;
    asm volatile ("sgdt %0" :: "m"(gdtr));

    uint32_t current_core_acpi_id = lapic_read(LAPIC_ID_REG);

    vmm_map_page(&kernel_mapping, 0, 0, 0x3, 0x3 | (1 << 7));

    for(uint64_t i = 0; i < madt0.element_cnt; i++) {
        struct madt0 madt0_entry = *vec_search(struct madt0, madt0, i);
        uint32_t acpi_id = madt0_entry.acpi_ID; 

        struct core_local core_local = {    .pid = -1,
                                            .tid = -1,
                                            .core_index = i,
                                            .kernel_stack = pmm_alloc(2) + 0x2000 + HIGH_VMA,
                                            .page_map = &kernel_mapping
                                       }; 

        vec_push(struct core_local, core_locals, core_local);

        if(acpi_id == current_core_acpi_id) {
            wrmsr(MSR_GS_BASE, (size_t)vec_search(struct core_local, core_locals, i));
            continue;
        }

        if(madt0_entry.flags == 1) {
            prep_core_init(pmm_alloc(2) + 0x2000 + HIGH_VMA, vmm_get_pml4(), (uint64_t)bootstrap_core, (uint64_t)&idtr, (uint64_t)&gdtr, i);
            send_IPI(acpi_id, 0x500); // MT = 0b101 for init ipi
            send_IPI(acpi_id, 0x600 | 1); // MT = 0b11 for startup, vec = 1 for 0x1000
            ksleep(20);
        }
    }

    vmm_unmap_page(&kernel_mapping, 0, 0x3 | (1 << 7));
}


struct core_local *get_core_local(int index) {
    if(index == CURRENT_CORE) {
        asm volatile ("mov %%gs:0, %0" : "=r"(index));
    }
    
    return vec_search(struct core_local, core_locals, (size_t)index);
}

void spin_lock(void *ptr) {
    volatile uint64_t cnt = 0;
    while(__atomic_test_and_set((char*)ptr, __ATOMIC_ACQUIRE)) {
        if(++cnt == 0x10000000) {
            kprintf("[KDEBUG] Possible deadlock");
        }
    }
}

void spin_release(void *ptr) {
    __atomic_clear((char*)ptr, __ATOMIC_RELEASE);
}
