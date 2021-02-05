#include <acpi/madt.h>
#include <drivers/hpet.h>
#include <int/apic.h>

uint32_t lapic_read(uint16_t offset) {
    return *(volatile uint32_t*)(madt->lapic_addr + HIGH_VMA + offset);    
}

void lapic_write(uint16_t offset, uint32_t data) {
    *(volatile uint32_t*)(madt->lapic_addr + HIGH_VMA + offset) = data;
}

void send_IPI(uint8_t ap, uint32_t ipi) {
    lapic_write(LAPIC_ICRH, (ap << 24));
    lapic_write(LAPIC_ICRL, ipi);
}

uint32_t ioapic_read(uint64_t base, uint32_t reg) {
    *(volatile uint32_t*)(base + 16 + HIGH_VMA) = reg;
    return *(volatile uint32_t*)(base + 18 + HIGH_VMA);
}

void ioapic_write(uint64_t base, uint32_t reg, uint32_t data) {
    *(volatile uint32_t*)(base + HIGH_VMA) = reg;
    *(volatile uint32_t*)(base + 16 + HIGH_VMA) = data;
}

void write_redirection(uint32_t gsi, uint64_t data, madt1_t *madt1_entry) {
    uint32_t reg = ((gsi - madt1_entry->gsi_base) * 2) + 16;
    ioapic_write(madt1_entry->ioapic_addr, reg, (uint32_t)data);
    ioapic_write(madt1_entry->ioapic_addr, reg + 1, (uint32_t)(data >> 32));
}

uint64_t read_redirection(uint32_t gsi, madt1_t *madt1_entry) {
    uint32_t reg = ((gsi - madt1_entry->gsi_base) * 2) + 16;
    uint64_t data = (uint64_t)ioapic_read(madt1_entry->ioapic_addr, reg);
    return data | ((uint64_t)(ioapic_read(madt1_entry->ioapic_addr, reg + 1)) << 32);
}

void mask_GSI(uint32_t gsi) {
    madt1_t *madt1_entry = vec_search(madt1_t, madt1, 0);
    write_redirection(gsi,  read_redirection(gsi, madt1_entry) | (1 << 16), madt1_entry);
}

void unmask_GSI(uint32_t gsi) {
    madt1_t *madt1_entry = vec_search(madt1_t, madt1, 0);
    write_redirection(gsi, read_redirection(gsi, 0) & (1 << 16), madt1_entry);
    write_redirection(gsi, gsi + 32, madt1_entry);
}

uint32_t get_max_GSI(uint64_t ioapic_base) {
    uint32_t data = ioapic_read(ioapic_base, 1) >> 16;
    return data & ~(1 << 7);
}

void lapic_timer_init(uint64_t ms) {
    lapic_write(LAPIC_TIMER_DIVIDE_CONF, 0x3);
    lapic_write(LAPIC_TIMER_INITAL_COUNT, ~(uint32_t)0); 

    ksleep(ms);

    uint32_t ticks = ~(uint32_t)0 - lapic_read(LAPIC_TIMER_CURRENT_COUNT);

    lapic_write(LAPIC_TIMER_LVT, 32 | 0x20000);
    lapic_write(LAPIC_TIMER_DIVIDE_CONF, 0x3);
    lapic_write(LAPIC_TIMER_INITAL_COUNT, ticks); 
}

void apic_init() {
    outb(0x20, 0x11);
    outb(0xa0, 0x11);
    outb(0x21, 0x20);
    outb(0xa1, 0x28);
    outb(0x21, 0x4);
    outb(0xa1, 0x2);
    outb(0x21, 0x1);
    outb(0xa1, 0x1);
    outb(0x21, 0x0);
    outb(0xa1, 0x0);

    outb(0xa1, 0xff); 
    outb(0x21, 0xff);

    for(uint8_t i = 0; i < madt1.element_cnt; i++) {
        madt1_t madt1_entry = *vec_search(madt1_t, madt1, i);
        for(uint32_t j = madt1_entry.gsi_base; j < get_max_GSI(madt1_entry.ioapic_addr); j++) {
            mask_GSI(j);  
        }
    }

    lapic_write(LAPIC_SINT, lapic_read(LAPIC_SINT) | 0x1ff); // enable spurious interrupts

    asm volatile("mov %0, %%cr8" :: "r"((uint64_t)0));
}
