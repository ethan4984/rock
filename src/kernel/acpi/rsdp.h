#pragma once

#include <kernel/mm/virtualPageManager.h>
#include <lib/stringUtils.h>
#include <lib/output.h>

#include <stdint.h>
#include <stddef.h>

struct [[gnu::packed]] acpihdr_t {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char OEMID[6];
    char OEMtableid[8];
    uint32_t OEMRevision;
    uint32_t creatorID;
    uint32_t creatorRevision;
};

struct [[gnu::packed]] rsdp_t {
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision; 
    uint32_t rsdtAddr;
    uint32_t length;
    uint64_t xsdtAddr;
    uint8_t extChecksum;
    uint8_t reserved[3];
};

struct [[gnu::packed]] rsdt_t {
    acpihdr_t acpihdr;
    uint32_t acpiptr[];
};

struct [[gnu::packed]] xsdt_t {
    acpihdr_t acpihdr;
    uint64_t acpiptr[];
};

namespace acpi {

inline rsdp_t *rsdp = NULL;
inline rsdt_t *rsdt = NULL;
inline xsdt_t *xsdt = NULL;

void rsdpInit(uint64_t *rsdpAddr);

template<typename T>
T *findSDT(const char *signature) {
    if(xsdt != NULL) {
        for(uint64_t i = 0; i < (xsdt->acpihdr.length - sizeof(acpihdr_t)); i++) {
            acpihdr_t *acpihdr = (acpihdr_t*)(xsdt->acpiptr[i] + HIGH_VMA);
            if(strncmp(acpihdr->signature, signature, 4) == 0) {
                cout + "[ACPI]" << signature << " located at " << (uint64_t)acpihdr << "\n";
                return (T*)acpihdr;
            }
        }
    } 

    if(rsdt != NULL) {
        for(uint64_t i = 0; i < (rsdt->acpihdr.length - sizeof(acpihdr_t)); i++) {
            acpihdr_t *acpihdr = (acpihdr_t*)((uint64_t)rsdt->acpiptr[i] + HIGH_VMA);
            if(strncmp(acpihdr->signature, signature, 4) == 0) {
                cout + "[ACPI]" << signature << " located at " << (uint64_t)acpihdr << "\n";
                return (T*)acpihdr;
            }
        }
    }

    cout + "[ACPI]" << signature << " cout not be found :(\n";

    return NULL;
}

};
