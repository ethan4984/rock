#include <acpi.h>
#include <memory.h>
#include <string.h>
#include <shitio.h>
#include <alloc.h>
#include <interrupt.h>

using namespace standardout;

rsdp_t *rsdp;
rsdt_t *rsdt;
xsdt_t *xsdt;
fadt_t *fadt;
madt_t *madt;
madt_info_t madt_info;

void *find_sdt(const char *signature);

madt_info_t::madt_info_t()
{
    cores = (madt0_t*)malloc(sizeof(madt0_t) * 4);
    ioapic = (madt1_t*)malloc(sizeof(madt1_t));
    iso = (madt2_t*)malloc(sizeof(madt2_t) * 5);
    non_maskable_int = (madt4_t*)malloc(sizeof(madt4_t));
    lapic_override = (madt5_t*)malloc(sizeof(madt5_t));
}

void madt_info_t::new_core(madt0_t *core)
{
    static uint64_t max_cores = 4;
    madt0_t entry;
    entry.processor_id = core->processor_id;
    entry.core_id = core->core_id;
    entry.flags = core->flags;
    if(max_cores + 1 == core_count) {
        free(cores);
        cores = (madt0_t*)malloc(sizeof(madt0_t) * (4 * (max_cores / 4 + 1)));
        max_cores += 4;
    }
    cores[core_count++] = entry;
}

void madt_info_t::new_iso(madt2_t *iso_t)
{
    static uint64_t max_iso = 5;
    madt2_t entry;
    entry.bus_src = iso_t->bus_src;
    entry.irq_src = iso_t->irq_src;
    entry.gsi = iso_t->gsi;
    entry.flags = iso_t->flags;
    if(max_iso + 1 == iso_count) {
        free(iso);
        iso = (madt2_t*)malloc(sizeof(madt2_t) * (iso_count));
        max_iso += 5;
    }
    iso[iso_count++] = entry;
}

void madt_info_t::cpu_info()
{
    k_print("\nCore count: %d\n", core_count);
    for(int i = 0; i < core_count; i++) {
        t_print("Core %d\n", i + 1);
        t_print("\tprocessor id: %d", cores[i].processor_id);
        t_print("\tcore id: %d", cores[i].core_id);
        t_print("\tflags: %d\n", cores[i].flags);
    }
}

void madt_info_t::iso_info()
{
    for(int i = 0; i < iso_count; i++) {
        t_print("ISO:\n");
        t_print("\tbus: %d", (uint32_t)iso[i].bus_src);
        t_print("\tIRQ: %d", (uint32_t)iso[i].irq_src);
        t_print("\tGSI: %d", (uint32_t)iso[i].gsi);
        t_print("\tFlags: %x\n", (uint32_t)iso[i].flags);
    }
}

uint8_t checksum()
{
    uint8_t sum = 0;
    for(uint32_t i = 0; i < rsdp->length; i++)
        sum += ((char*)rsdp)[i];
    return sum;
}

void init_acpi()
{
    k_print("ACPI init:\n\n");
    bool found_something = false;
    for(uint64_t i = kernel_high + 0xe0000; i < kernel_high + 0x100000; i += 16) { // looks for the rsdp (root system description pointer
        if(!strncmp((char*)i, "RSD PTR ", 8)) {
            rsdp = (rsdp_t*)i; // fill the tables

            uint8_t sum = checksum();

            if(rsdp->xsdtAddr) { // only on real hardware or a virtualizer that supports ACPI 2.0
                xsdt = (xsdt_t*)((uint32_t)rsdp->xsdtAddr);
                k_print("\tACPI: XSDT found at %x\n", xsdt);
            }
            else {
                rsdt = (rsdt_t*)((uint32_t)rsdp->rsdtAddr);
                k_print("\tACPI: RSDP found at %x\n", rsdt);
            }
            found_something = true;
            break;
        }
    }
    if(!found_something)
        k_print("\tACPI: No RSDP found\n");

    fadt = (fadt_t*)find_sdt("FACP");
    madt = (madt_t*)find_sdt("APIC");

    init_madt();
}

void *find_sdt(const char *signature) // find any sdt (system discriptor table)
{
    if(xsdt == NULL) {
        for(uint64_t i = 0; i < (rsdt->ACPI_header.length - sizeof(ACPI_header_t)) / 4; i++) {
            ACPI_header_t *header = (ACPI_header_t*)rsdt->ACPI_hptr[i];
            if(!strncmp(header->signature, signature, 4)) {
                k_print("\tACPI: %s found at %x\n", signature, header);
                return (void*)header;
            }
        }
    }
    else { // ok this machien is xsdt
        for(uint64_t i = 0; i < (rsdt->ACPI_header.length - sizeof(ACPI_header_t)) / 4; i++) {
            ACPI_header_t *header = (ACPI_header_t*)xsdt->ACPI_hptr[i];
            if(!strncmp(header->signature, signature, 4)) {
                k_print("\tACPI: %s found at %x\n", signature, header);
                return (void*)header;
            }
        }
    }
    t_print("ACPI: %s not found", signature);
    return NULL;
}

void init_madt()
{
    k_print("\nMADT init: \n\n");
    madt_info.lapic = (uint64_t)madt->lapic_addr;
    k_print("lapic base addr: %x\n", madt_info.lapic);
    uint64_t entry_byte_size = madt->ACPI_header.length - (sizeof(madt->ACPI_header) + sizeof(madt->lapic_addr) + sizeof(madt->flags));
    for(int i = 0; i < entry_byte_size; i++) {
        uint8_t entry_type = madt->entries[i++];
        uint8_t entry_size = madt->entries[i++];

        switch(entry_type) {
            case 0: {
                madt0_t *entry = (madt0_t*)&(madt->entries[i]);
                madt_info.new_core(entry);
                break;
            }
            case 1: {
                madt1_t *entry = (madt1_t*)&(madt->entries[i]);
                t_print("IOAPIC:\n");
                t_print("\tAddr: %x\n", (uint32_t)entry->ioapic_addr);
                break;
            }
            case 2: {
                madt2_t *entry = (madt2_t*)&(madt->entries[i]);
                madt_info.new_iso(entry);
                break;
            }
            case 4: {
                madt4_t *entry = (madt4_t*)&(madt->entries[i]);
                t_print("None Maskable Interrupt on:\n");
                t_print("\tcore ID: %d", (uint32_t) entry->processor_id);
                t_print("\tLINT: %d", (uint32_t) entry->lint);
                t_print("\tFlags: %d\n", (uint32_t) entry->flags);
                break;
            }
            case 5: {
                madt5_t *entry = (madt5_t*)&(madt->entries[i]);
                t_print("LAPIC override:\n");
                t_print("New addr: %x\n", entry->lapic_override);
                madt_info.lapic = entry->lapic_override;
                break;
            }
        }
        i += entry_size - 3;
    }
    madt_info.iso_info();
    madt_info.cpu_info();
}
