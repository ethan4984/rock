#include <kernel/acpi/madt.h>
#include <lib/output.h>

namespace kernel {

void madtInfo_t::madtInit() {
    madt_t *madt = (madt_t*)acpi.findSDT("APIC");

    lapicAddr = madt->lapicAddr;

    cout + "[APIC]" << lapicAddr << "\n";

    madtEntry0 = new madtEntry0_t[50];
    madtEntry1 = new madtEntry1_t[50];
    madtEntry2 = new madtEntry2_t[50];
    madtEntry4 = new madtEntry4_t[50];
    madtEntry5 = new madtEntry5_t[50];

    for(uint64_t i = 0; i < madt->acpihdr.length - (sizeof(madt->acpihdr) + sizeof(madt->lapicAddr) + sizeof(madt->flags)); i++) {
        uint8_t entryType = madt->entries[i++];
        uint8_t entrySize = madt->entries[i++];

        switch(entryType) {
            case 0:
                madtEntry0[madtInfo.madtEntry0Count++] = ((madtEntry0_t*)&(madt->entries[i]))[0];
                break;
            case 1:
                madtEntry1[madtInfo.madtEntry1Count++] = ((madtEntry1_t*)&(madt->entries[i]))[0];
                break;
            case 2:
                madtEntry2[madtInfo.madtEntry2Count++] = ((madtEntry2_t*)&(madt->entries[i]))[0];
                break;
            case 4:
                madtEntry4[madtInfo.madtEntry4Count++] = ((madtEntry4_t*)&(madt->entries[i]))[0];
                break;
            case 5:
                madtEntry5[madtInfo.madtEntry5Count++] = ((madtEntry5_t*)&(madt->entries[i]))[0];
        }
        i += entrySize - 3;
    }
}

void madtInfo_t::printMADT() {
    for(uint64_t i = 0; i < madtInfo.madtEntry0Count; i++) {
        cout + "[APIC]" << "Parsing madt0 entry " << i << "\n";
        cout + "[APIC]" << "Processor " << i << "s lapic\n";
        cout + "[APIC]" << "ACPI processor ID: " << madtInfo.madtEntry0[i].acpiProcessorID << "\n";
        cout + "[APIC]" << "APIC ID: " << madtInfo.madtEntry0[i].apicID << "\n";
        cout + "[APIC]" << "Flags: " << madtInfo.madtEntry0[i].flags << "\n";
    }

    for(uint64_t i = 0; i < madtInfo.madtEntry1Count; i++) {
        cout + "[APIC]" << "Parsing madt1 entry " << i << "\n";
        cout + "[APIC]" << "I/O APIC ID: " << madtInfo.madtEntry1[i].ioapicID << "\n";
        cout + "[APIC]" << "I/O APIC address " << madtInfo.madtEntry1[i].ioapicAddr << "\n";
        cout + "[APIC]" << "GSI base " << madtInfo.madtEntry1[i].gsiBase << "\n";
    }

    for(uint64_t i = 0; i < madtInfo.madtEntry2Count; i++) {
        cout + "[APIC]" << "Parsing madt2 entry " << i << "\n";
        cout + "[APIC]" << "bus source: " << madtInfo.madtEntry2[i].busSrc << "\n";
        cout + "[APIC]" << "irq source: " << madtInfo.madtEntry2[i].irqSrc << "\n";
        cout + "[APIC]" << "gsi " << madtInfo.madtEntry2[i].gsi << "\n";
        cout + "[APIC]" << "flags " << madtInfo.madtEntry2[i].flags << "\n";
    }

    for(uint64_t i = 0; i < madtInfo.madtEntry4Count; i++) {
        cout + "[APIC]" << "Parsing madt4 entry " << i << "\n";
        cout + "[APIC]" << "ACPI processor ID " << madtInfo.madtEntry4[i].acpiProcessorID << "\n";
        cout + "[APIC]" << "flags " << madtInfo.madtEntry4[i].flags << "\n";
        cout + "[APIC]" << "lint " << madtInfo.madtEntry4[i].lint << "\n";
    }

    for(uint64_t i = 0; i < madtInfo.madtEntry5Count; i++) {
        cout + "[APIC]" << "Parsing madt5 entry " << i << "\n";
        cout + "[APIC]" << "lapic override " << madtInfo.madtEntry5[i].lapicOverride << "\n";
    }
}

}
