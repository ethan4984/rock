#ifndef MADT_H_
#define MADT_H_

#include <acpi/tables.h>
#include <vec.h>

typedef struct {
    acpihdr_t acpihdr;
    uint32_t lapic_addr;
    uint32_t flags;
    uint8_t entries[];
} __attribute__((packed)) madt_t;

typedef struct {
    uint8_t acpi_ID;
    uint8_t apic_ID;
    uint32_t flags;
} __attribute__((packed)) madt0_t;

typedef struct {
    uint8_t ioapic_ID;
    uint8_t reserved;
    uint32_t ioapic_addr;
    uint32_t gsi_base;
} __attribute__((packed)) madt1_t;

typedef struct {
    uint8_t bus_src;
    uint8_t irq_src;
    uint32_t gsi;
    uint16_t flags;
} __attribute__((packed)) madt2_t;

typedef struct {
    uint8_t acpi_ID;
    uint16_t flags; 
    uint8_t lint;
} __attribute__((packed)) madt4_t;

typedef struct {
    uint16_t reserved;
    uint8_t lapic_override;
} __attribute__((packed)) madt5_t;

extern madt_t *madt;

vec_create(madt0_t, madt0);
extern_vec(madt0);

vec_create(madt1_t, madt1);
extern_vec(madt1);

vec_create(madt2_t, madt2);
extern_vec(madt2);

vec_create(madt4_t, madt4);
extern_vec(madt4);

vec_create(madt5_t, madt5);
extern_vec(madt5);

void madt_init();

#endif
