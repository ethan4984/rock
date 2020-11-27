#ifndef MADT_H_
#define MADT_H_

#include <acpi/tables.h>

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

typedef struct { 
    uint8_t ent0cnt;
    uint8_t ent1cnt;
    uint8_t ent2cnt;
    uint8_t ent4cnt;
    uint8_t ent5cnt;

    madt0_t *ent0;
    madt1_t *ent1;
    madt2_t *ent2;
    madt4_t *ent4;
    madt5_t *ent5;

    uint32_t lapic_addr;
} madt_info_t;

extern madt_info_t madt_info;

void madt_init();

#endif
