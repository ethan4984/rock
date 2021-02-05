#include <sched/smp.h>
#include <bitmap.h> 
#include <mm/vmm.h>
#include <int/gdt.h>
#include <int/syscall.h>
#include <output.h>
#include <mm/pmm.h>
#include <drivers/hpet.h>
#include <acpi/madt.h>
#include <memutils.h>
#include <int/idt.h>
#include <asmutils.h>

extern symbol smp_tramp_begin; 
extern symbol smp_tramp_end;

static uint64_t core_cnt = 0;
static core_local_t *core_local;

static void prep_trampoline(uint64_t stack, uint64_t pml4, uint64_t entry_point, uint64_t idt) {
    uint64_t *parameters = (uint64_t*)(0x500 + HIGH_VMA);
    parameters[0] = stack;
    parameters[1] = pml4;
    parameters[2] = entry_point;
    parameters[3] = idt;
}

static void init_cpu_features() {
    wrmsr(MSR_EFER, rdmsr(MSR_EFER) | 1); // set SCE

    wrmsr(MSR_STAR, 0x0013000800000000);
    wrmsr(MSR_LSTAR, (uint64_t)syscall_main_stub);
    wrmsr(MSR_SFMASK, (uint64_t)~((uint32_t)0x002));

    set_kernel_gs((uint64_t)&core_local[core_cnt]);
}

static void bootstrap_core() {
    init_cpu_features();

    lapic_write(LAPIC_SINT, lapic_read(LAPIC_SINT) | 0x1ff);

    create_generic_tss();

    lapic_timer_init(50);

    for(;;)
        asm ("pause");
}

void init_smp() {
    core_local = kmalloc(sizeof(core_local_t) * madt0.element_cnt);
    for(uint8_t i = 0; i < madt0.element_cnt; i++) {
        core_local[i] = (core_local_t) {    .pid = -1,
                                            .tid = -1,
                                            .core_index = i,
                                            .kernel_stack = pmm_alloc(4) + 0x4000 + HIGH_VMA,
                                            .pagestruct = &kernel_mapping
                                       };
    }

    init_cpu_features();

    memcpy8((uint8_t*)(0x1000 + HIGH_VMA), (uint8_t*)smp_tramp_begin, (uint64_t)smp_tramp_end - (uint64_t)smp_tramp_begin);

    static idtr_t idtr;
    asm volatile ("sidt %0" :: "m"(idtr));

    for(uint64_t i = 1; i < madt0.element_cnt; i++) {
        madt0_t madt0_entry = *vec_search(madt0_t, madt0, i);
        uint32_t coreID = madt0_entry.apic_ID;

        kprintf("[SMP]", "Starting up core %d", i); 
        kvprintf("[SMP] Starting up core %d\n", i); 

        if(madt0_entry.flags == 1) {
            prep_trampoline(pmm_alloc(2) + 0x2000 + HIGH_VMA, grab_PML4(), (uint64_t)bootstrap_core, (uint64_t)&idtr);
            send_IPI(coreID, 0x500); // MT = 0b101 for init ipi
            send_IPI(coreID, 0x600 | 1); // MT = 0b11 for startup, vec = 1 for 0x1000
            ksleep(20); 
        }
    }
}

core_local_t *get_core_local(int64_t index) {
    if(index == -1) {
        asm volatile ("mov %%gs:0, %0" : "=r"(index));
    }
    return &core_local[index];
}

void spin_lock(void *ptr) {
    volatile uint64_t cnt = 0;
    while(__atomic_test_and_set((char*)ptr, __ATOMIC_ACQUIRE)) {
        if(++cnt == 0x10000000) {
            kprintf("[KDEBUG]", "Possible deadlock");
        }
    }
}

void spin_release(void *ptr) {
    __atomic_clear((char*)ptr, __ATOMIC_RELEASE);
}
