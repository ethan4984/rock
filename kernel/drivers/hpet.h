#ifndef HPET_H_
#define HPET_H_

#include <acpi/rsdp.h>

struct hpet_table {
    struct acpihdr acpihdr;
    uint8_t hardware_rev_ID;
    uint8_t info;
    uint16_t pci_ID;
    uint8_t address_space_ID;
    uint8_t register_width;
    uint8_t register_offset;
    uint8_t reserved;
    uint64_t address;
    uint8_t hpet_num;
    uint16_t minim_ticks;
    uint8_t page_protection;
} __attribute__((packed));

struct hpet {
    uint64_t capabilities;
    uint64_t unused0;
    uint64_t general_config;
    uint64_t unused1;
    uint64_t int_status;
    uint64_t unused2;
    uint64_t unused3[24];
    uint64_t counter_value;
    uint64_t unused4;
} __attribute__((packed));

void ksleep(size_t ms);
void init_hpet();

#endif
