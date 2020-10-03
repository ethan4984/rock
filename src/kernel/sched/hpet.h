#pragma once

#include <kernel/acpi/rsdp.h>

typedef struct {
    acpihdr_t acpihdr;     
    uint8_t hardwareRevID;
    uint8_t info;
    uint16_t pciID;
    uint8_t addressSpaceID; 
    uint8_t registerWidth;
    uint8_t registerOffset;
    uint8_t reserved;
    uint64_t address;
    uint8_t hpetNum;
    uint16_t miniumTicks;
    uint8_t pageProtection;
} __attribute__((packed)) hpetTable_t;

typedef struct {
    uint64_t capabilities;
    uint64_t unused0;
    uint64_t generalConfig;
    uint64_t unused1;
    uint64_t intStatus;
    uint64_t unused2;
    uint64_t unused3[24];
    uint64_t counterValue;
    uint64_t unused4;
} __attribute__((packed)) hpet_t;

void initHPET();

void ksleep(uint64_t ms);
