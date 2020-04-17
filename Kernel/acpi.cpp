#include <acpi.h>
#include <memory.h>
#include <string.h>
#include <shitio.h>
#include <alloc.h>
#include <interrupt.h>

using namespace standardout;

rsdp_t* rsdp;
rsdt_t* rsdt;
xsdt_t* xsdt;
fadt_t* fadt;
madt_t* madt;

void *find_sdt(const char *signature);

uint8_t checksum()
{
    uint8_t sum = 0;
    for(uint32_t i = 0; i < rsdp->length; i++)
        sum += ((char*)rsdp)[i];
    return sum;
}

void init_acpi()
{
    bool rsdp_found = false;
    for(uint64_t i = kernel_high + 0xe0000; i < kernel_high + 0x100000; i += 16) { // looks for the rsdp (root system description pointer
        if(!strncmp((char*)i, "RSD PTR ", 8)) {
            rsdp = (rsdp_t*)i; // fill the tables

            uint8_t sum = checksum();

            if(rsdp->revision == 2 && rsdp->xsdtAddr) // only on real hardware or a virtualizer that supports ACPI 2.0
                xsdt = (xsdt_t*)((uint32_t)rsdp->xsdtAddr);
            else
                rsdt = (rsdt_t*)((uint32_t)rsdp->rsdtAddr);
            rsdp_found = true;
            t_print("\nACPI: RSDP found at %x\n", rsdt);
            break;
        }
    }
    if(!rsdp_found)
        t_print("ACPI: No RSDP found");

    fadt = (fadt_t*)find_sdt("FACP");
    madt = (madt_t*)find_sdt("APIC");
}

void *find_sdt(const char *signature) // find any sdt (system discriptor table
{
    for(uint64_t i = 0; i < (rsdt->ACPI_header.length - sizeof(ACPI_header_t)) / 4; i++) {
        ACPI_header_t *header = (ACPI_header_t*)rsdt->ACPI_hptr[i];
        if(!strncmp(header->signature, signature, 4)) {
            t_print("ACPI: %s", signature);
            t_print("found at %x\n", header);
            return (void*)header;
        }
    }
    t_print("ACPI: %s not found");
    return NULL;
}
