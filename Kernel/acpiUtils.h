#pragma once 

#include <stdint.h>

void initAcpi();

void *findSDT(const char *signature);

class ACPIheader_t 
{
    public:
        char signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        char OEMID[6];
        char OEMTABLEID[8];
        uint32_t oemRevision;
        uint32_t creatorID;
        uint32_t creatorRevision;
} __attribute__((packed));

class rsdp_t /* the root system decirptor table points either the rsd(table) or the xsdt */
{
    public:
        char signature[8];
        uint8_t checksum;
        char OEMID[6];
        uint8_t revision;
        uint32_t rsdtAddr;
        uint32_t length;
        uint64_t xsdtAddr;
        uint8_t extChecksum;
        uint8_t reserved[3];
} __attribute__((packed));

class rsdt_t
{
    public:
        ACPIheader_t ACPIheader;
        uint32_t ACPIptr[];
} __attribute__((packed));

class xsdt_t
{
    public:
        ACPIheader_t ACPI_header;
        uint64_t ACPIptr[];
} __attribute__((packed));
