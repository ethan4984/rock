#ifndef APIC_H_
#define APIC_H_

#include <asmutils.h>
#include <mm/vmm.h>

#define MSR_APIC_BASE 0x1b

#define LAPIC_ID_REG 0x20
#define LAPIC_VERSION_REG 0x30
#define LAPIC_TPR 0x80
#define LAPIC_APR 0x90
#define LAPIC_PPR 0xa0
#define LAPIC_EOI 0xb0
#define LAPIC_REMOTE_READ_REG 0xc
#define LAPIC_LDR 0xd0
#define LAPIC_DFR 0xe0
#define LAPIC_SINT 0xf0
#define LAPIC_ISR0 0x100
#define LAPIC_ISR1 0x110
#define LAPIC_ISR2 0x120
#define LAPIC_ISR3 0x130
#define LAPIC_ISR4 0x140
#define LAPIC_ISR5 0x150
#define LAPIC_ISR6 0x160
#define LAPIC_ISR7 0x170
#define LAPIC_TMR0 0x180
#define LAPIC_TMR1 0x190
#define LAPIC_TMR2 0x1a0
#define LAPIC_TMR3 0x1b0
#define LAPIC_TMR4 0x1c0
#define LAPIC_TMR5 0x1d0
#define LAPIC_TMR6 0x1e0
#define LAPIC_TMR7 0x1f0
#define LAPIC_IRR0 0x200
#define LAPIC_IRR1 0x210
#define LAPIC_IRR2 0x220
#define LAPIC_IRR3 0x230
#define LAPIC_IRR4 0x240
#define LAPIC_IRR5 0x250
#define LAPIC_IRR6 0x260
#define LAPIC_IRR7 0x270
#define LAPIC_ESR 0x280
#define LAPIC_ICRL 0x300
#define LAPIC_ICRH 0x310
#define LAPIC_TIMER_LVT 0x320
#define LAPIC_THERMAL_LVT 0x330
#define LAPIC_PERFORMANCE_LVT 0x340
#define LAPIC_LINT0 0x350
#define LAPIC_LINT2 0x360
#define LAPIC_EVT 0x370
#define LAPIC_TIMER_INITAL_COUNT 0x380
#define LAPIC_TIMER_CURRENT_COUNT 0x390
#define LAPIC_TIMER_DIVIDE_CONF 0x3e0
#define LAPIC_EAPIC_FEATURE 0x400
#define LAPIC_EAPIC_CONTROL 0x410
#define LAPIC_SEOI 0x420
#define LAPIC_IER0 0x480
#define LAPIC_IER1 0x490
#define LAPIC_IER2 0x4a0
#define LAPIC_IER3 0x4b0
#define LAPIC_IER4 0x4c0
#define LAPIC_IER5 0x4d0
#define LAPIC_IER6 0x4e0
#define LAPIC_IER7 0x4f0
#define LAPIC_ELVT0 0x500
#define LAPIC_ELVT1 0x510
#define LAPIC_ELVT2 0x520
#define LAPIC_ELVT3 0x530

void apic_init();

void lapic_write(uint16_t offset, uint32_t data);

uint32_t lapic_read(uint16_t offset);

void send_IPI(uint8_t ap, uint32_t ipi);

void lapic_timer_init(uint64_t ms);

#endif
