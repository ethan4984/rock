#include <acpi/madt.h>
#include <acpi/rsdp.h>
#include <debug.h>
#include <vec.h>

global_vec(madt0);
global_vec(madt1);
global_vec(madt2);
global_vec(madt4);
global_vec(madt5);

struct madt *madt;

void madt_init() {
    madt = find_SDT("APIC");

    for(uint64_t i = 0; i < madt->acpihdr.length - (sizeof(madt->acpihdr) + sizeof(madt->lapic_addr) + sizeof(madt->flags)); i++) {
        uint8_t entry_type = madt->entries[i++];
        uint8_t entry_size = madt->entries[i++];

        switch(entry_type) {
            case 0:
                vec_push(struct madt0, madt0, *(struct madt0*)&(madt->entries[i]));
                break;
            case 1:
                vec_push(struct madt1, madt1, *(struct madt1*)&(madt->entries[i]));
                break;
            case 2:
                vec_push(struct madt2, madt2, *(struct madt2*)&(madt->entries[i]));
                break;
            case 4:
                vec_push(struct madt4, madt4, *(struct madt4*)&(madt->entries[i]));
                break;
            case 5:
                vec_push(struct madt5, madt5, *(struct madt5*)&(madt->entries[i]));
        }
        i += entry_size - 3;
    }

    kprintf("[ACPI] core count detected %d\n", madt0.element_cnt);
}
