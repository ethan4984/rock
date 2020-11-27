#include <acpi/madt.h>
#include <acpi/rsdp.h>
#include <output.h>
#include <bitmap.h>

madt_info_t madt_info;

void madt_init() {
    madt_t *madt = find_SDT("APIC");

    madt_info.lapic_addr = madt->lapic_addr;

    madt_info.ent0 = kmalloc(sizeof(madt0_t) * 50);
    madt_info.ent1 = kmalloc(sizeof(madt1_t) * 50);
    madt_info.ent2 = kmalloc(sizeof(madt2_t) * 50);
    madt_info.ent4 = kmalloc(sizeof(madt4_t) * 50);
    madt_info.ent5 = kmalloc(sizeof(madt5_t) * 50);

    for(uint64_t i = 0; i < madt->acpihdr.length - (sizeof(madt->acpihdr) + sizeof(madt->lapic_addr) + sizeof(madt->flags)); i++) {
        uint8_t entry_type = madt->entries[i++];
        uint8_t entry_size = madt->entries[i++];

        switch(entry_type) {
            case 0:
                madt_info.ent0[madt_info.ent0cnt++] = ((madt0_t*)&(madt->entries[i]))[0];
                break;
            case 1:
                madt_info.ent1[madt_info.ent1cnt++] = ((madt1_t*)&(madt->entries[i]))[0];
                break;
            case 2:
                madt_info.ent2[madt_info.ent2cnt++] = ((madt2_t*)&(madt->entries[i]))[0];
                break;
            case 4:
                madt_info.ent4[madt_info.ent4cnt++] = ((madt4_t*)&(madt->entries[i]))[0];
                break;
            case 5:
                madt_info.ent5[madt_info.ent5cnt++] = ((madt5_t*)&(madt->entries[i]))[0];
        }
        i += entry_size - 3;
    }

    kvprintf("[ACPI] core count detected %d\n", madt_info.ent0cnt);
}
