#pragma once

#include <stdint.h>
#include <acpi.h>

#define kernel_high 0xffffffff80000000

void init_acpi();
void init_madt();

typedef struct
{
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char OEMID[6];
    char OEMTABLEID[8];
    uint32_t oemRevision;
    uint32_t creatorID;
    uint32_t creatorRevision;
} __attribute__((packed)) ACPI_header_t;

typedef struct
{
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t rsdtAddr;
    uint32_t length;
    uint64_t xsdtAddr;
    uint8_t extChecksum;
    uint8_t reserved[3];
} __attribute__((packed)) rsdp_t;

typedef struct
{
    ACPI_header_t ACPI_header;
    uint32_t ACPI_hptr[];
} __attribute__((packed)) rsdt_t;

typedef struct
{
    ACPI_header_t ACPI_header;
    uint32_t firmwarer_control;
    uint32_t dsdt;
    uint8_t reserved;
    uint8_t PPMP; // Preferred Power Management Profile
    uint16_t SMI_int;
    uint32_t SMT_command_port;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstat_control;
    // fill the rest
} __attribute__((packed)) fadt_t;

typedef struct
{
    ACPI_header_t ACPI_header;
    uint64_t ACPI_hptr[];
} __attribute__((packed)) xsdt_t;

typedef struct
{
    ACPI_header_t ACPI_header;
    uint32_t lapic_addr;
    uint32_t flags;
    uint8_t entries[];
} __attribute__((packed)) madt_t;

typedef struct {
    uint8_t processor_id;
    uint8_t core_id;
    uint32_t flags;
} __attribute__ ((packed)) madt0_t;

typedef struct {
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t ioapic_addr;
    uint32_t gsi_base;
} __attribute__ ((packed)) madt1_t;

typedef struct {
    uint8_t bus_src;
    uint8_t irq_src;
    uint32_t gsi;
    uint16_t flags;
} __attribute__ ((packed)) madt2_t;

typedef struct {
    uint8_t processor_id;
    uint16_t flags;
    uint8_t lint;
} __attribute__ ((packed)) madt4_t;

typedef struct {
    uint16_t reserved;
    uint64_t lapic_override;
} __attribute__ ((packed)) madt5_t;

class madt_info_t
{
    public:
        madt_info_t();

        uint64_t lapic;
        uint64_t core_count = 0;
        uint64_t ioapic_count = 0;
        uint64_t iso_count = 0;
        uint64_t nmi_count = 0;
        uint64_t lapic_override_count = 0;

        madt0_t *cores;
        madt1_t *ioapic;
        madt2_t *iso;
        madt4_t *non_maskable_int;
        madt5_t *lapic_override;

        void new_core(madt0_t *core);
        void new_iso(madt2_t *iso_t);
        void new_nmi(madt4_t *nmi);
        void new_lapic_override(madt5_t *lapic_override);

        void cpu_info();
        void iso_info();
};

extern rsdp_t *rsdp;
extern rsdt_t *rsdt;
extern xsdt_t *xsdt;
extern fadt_t *fadt;
extern madt_t *madt;
extern madt_info_t madt_info;
