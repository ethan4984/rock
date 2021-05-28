#ifndef APIC_HPP_
#define APIC_HPP_

#include <cpu.hpp>
#include <acpi/madt.hpp>

namespace apic {

constexpr size_t msr_lapic_base = 0x1b;

constexpr size_t id_reg =  0x20;
constexpr size_t version_reg = 0x30;
constexpr size_t tpr =  0x80;
constexpr size_t apr = 0x90;
constexpr size_t ppr =  0xa0;
constexpr size_t eoi = 0xb0;
constexpr size_t remote_read_reg = 0xc;
constexpr size_t ldr = 0xd0;
constexpr size_t dfr = 0xe0;
constexpr size_t sint = 0xf0;
constexpr size_t isr0 = 0x100;
constexpr size_t isr1 = 0x110;
constexpr size_t isr2 = 0x120;
constexpr size_t isr3 = 0x130;
constexpr size_t isr4 = 0x140;
constexpr size_t isr5 = 0x150;
constexpr size_t isr6 = 0x160;
constexpr size_t isr7 = 0x170;
constexpr size_t tmr0 = 0x180;
constexpr size_t tmr1 = 0x190;
constexpr size_t tmr2 = 0x1a0;
constexpr size_t tmr3 = 0x1b0;
constexpr size_t tmr4 = 0x1c0;
constexpr size_t tmr5 = 0x1d0;
constexpr size_t tmr6 = 0x1e0;
constexpr size_t tmr7 = 0x1f0;
constexpr size_t irr0 = 0x200;
constexpr size_t irr1 = 0x210;
constexpr size_t irr2 = 0x220;
constexpr size_t irr3 = 0x230;
constexpr size_t irr4 = 0x240;
constexpr size_t irr5 = 0x250;
constexpr size_t irr6 = 0x260;
constexpr size_t irr7 = 0x270;
constexpr size_t esr = 0x280;
constexpr size_t icrl = 0x300;
constexpr size_t icrh = 0x310;
constexpr size_t timer_lvt = 0x320;
constexpr size_t thermal_lvt = 0x330;
constexpr size_t performance_lvt = 0x340;
constexpr size_t lint0 = 0x350;
constexpr size_t lint2 = 0x360;
constexpr size_t evt = 0x370;
constexpr size_t timer_inital_count = 0x380;
constexpr size_t timer_current_count =  0x390;
constexpr size_t timer_divide_conf = 0x3e0;
constexpr size_t eapic_feature = 0x400;
constexpr size_t eapic_control = 0x410;
constexpr size_t seoi = 0x420;
constexpr size_t ier0 = 0x480;
constexpr size_t ier1 = 0x490;
constexpr size_t ier2 = 0x4a0;
constexpr size_t ier3 = 0x4b0;
constexpr size_t ier4 = 0x4c0;
constexpr size_t ier5 = 0x4d0;
constexpr size_t ier6 = 0x4e0;
constexpr size_t ier7 = 0x4f0;
constexpr size_t elvt0 = 0x500;
constexpr size_t elvt1 = 0x510;
constexpr size_t elvt2 = 0x520;
constexpr size_t elvt3 = 0x530;

class ioapic {
public:
    ioapic(madt1 madt1_ptr) : madt1_ptr(madt1_ptr) { }
    ioapic() = default;

    uint32_t read(uint32_t reg);
    void write(uint32_t reg, uint32_t data);

    size_t read_redirection(uint32_t gsi);
    void write_redirection(uint32_t gsi, size_t data);

    void mask_gsi(uint32_t gsi);
    void unmask_gsi(uint32_t gsi);
    void mask_all_gsi();
    void redirect_gsi(uint8_t vector, uint32_t gsi, uint16_t flags);

    uint32_t max_gsi();
private:
    friend ioapic ioapic_validate(uint32_t gsi);

    madt1 madt1_ptr;
};

uint32_t lapic_read(uint16_t reg);
void lapic_write(uint16_t reg, uint32_t data);
void send_ipi(uint8_t ap, uint32_t ipi);
void timer_calibrate(uint64_t ms);

void init();

}

#endif
