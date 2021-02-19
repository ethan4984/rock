#ifndef MADT_H_
#define MADT_H_

#include <acpi/tables.h>
#include <vec.h>

struct madt {
    struct acpihdr acpihdr;
    uint32_t lapic_addr;
    uint32_t flags;
    uint8_t entries[];
} __attribute__((packed));

struct madt0 {
    uint8_t acpi_ID;
    uint8_t apic_ID;
    uint32_t flags;
} __attribute__((packed));

struct madt1 {
    uint8_t ioapic_ID;
    uint8_t reserved;
    uint32_t ioapic_addr;
    uint32_t gsi_base;
} __attribute__((packed));

struct madt2 {
    uint8_t bus_src;
    uint8_t irq_src;
    uint32_t gsi;
    uint16_t flags;
} __attribute__((packed));

struct madt4 {
    uint8_t acpi_ID;
    uint16_t flags; 
    uint8_t lint;
} __attribute__((packed));

struct madt5 {
    uint16_t reserved;
    uint8_t lapic_override;
} __attribute__((packed));

extern struct madt *madt;

extern_vec(struct madt0, madt0);
extern_vec(struct madt1, madt1);
extern_vec(struct madt2, madt2);
extern_vec(struct madt4, madt4);
extern_vec(struct madt5, madt5);

void madt_init();

#endif
