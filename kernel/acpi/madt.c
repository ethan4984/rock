#include <acpi/madt.h>
#include <acpi/rsdp.h>
#include <output.h>
#include <bitmap.h>
#include <vec.h>

global_vec(madt0);
global_vec(madt1);
global_vec(madt2);
global_vec(madt4);
global_vec(madt5);

madt_t *madt;

void madt_init() {
    madt = find_SDT("APIC");

    for(uint64_t i = 0; i < madt->acpihdr.length - (sizeof(madt->acpihdr) + sizeof(madt->lapic_addr) + sizeof(madt->flags)); i++) {
        uint8_t entry_type = madt->entries[i++];
        uint8_t entry_size = madt->entries[i++];

        switch(entry_type) {
            case 0:
                vec_push(madt0_t, madt0, ((madt0_t*)&(madt->entries))[0]);
                break;
            case 1:
                vec_push(madt1_t, madt1, ((madt1_t*)&(madt->entries))[0]);
                break;
            case 2:
                vec_push(madt2_t, madt2, ((madt2_t*)&(madt->entries))[0]);
                break;
            case 4:
                vec_push(madt4_t, madt4, ((madt4_t*)&(madt->entries))[0]);
                break;
            case 5:
                vec_push(madt5_t, madt5, ((madt5_t*)&(madt->entries))[0]);
        }
        i += entry_size - 3;
    }

    kvprintf("[ACPI] core count detected %d\n", madt0.element_cnt);
}
