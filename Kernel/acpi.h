#pragma once

#include <stdint.h>

#define kernel_high 0xffffffff80000000

void init_acpi();

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

extern rsdp_t* rsdp;
extern rsdt_t* rsdt;
extern xsdt_t* xsdt;
extern fadt_t* fadt;
extern madt_t* madt;
