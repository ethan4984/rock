#include <Kernel/int/madt.h>
#include <Kernel/mm/memHandler.h>
#include <Slib/videoIO.h> 

using namespace out;

madtInfo_t madtInfo;

void madtParse()
{
    madt_t *madt = (madt_t*)findSDT("APIC");
    cPrint("\nMADT address found at %x", madt);

    madtInfo.lapicAddr = madt->lapicAddr;

    for(uint64_t i = 0; i < madt->ACPIheader.length - (sizeof(madt->ACPIheader) + sizeof(madt->lapicAddr) + sizeof(madt->flags)); i++) {
        uint8_t entry_type = madt->entries[i++];
        uint8_t entry_size = madt->entries[i++];

        switch(entry_type) {
            case 0: { /* tpye 0 = Processor local APIC */
                madt0_t *entry = (madt0_t*)&(madt->entries[i]);
                madtInfo.newMadt0Entry(entry);
                break;
            }
            case 1: { /* type 1 = I/O APIC */
                madt1_t *entry = (madt1_t*)&(madt->entries[i]);
                madtInfo.newMadt1Entry(entry);
                break;
            }
            case 2: { /* tpye 2 = ISO / Interrupt Source Override */
                madt2_t *entry = (madt2_t*)&(madt->entries[i]);
                madtInfo.newMadt2Entry(entry);
                break;
            }
            case 4: { /* type 4 = NMI / non-maskable interrupts */
                madt4_t *entry = (madt4_t*)&(madt->entries[i]);
                madtInfo.newMadt4Entry(entry);
                break;
            }
            case 5: { /* type 5 = lapic address override */
                madt5_t *entry = (madt5_t*)&(madt->entries[i]);
                madtInfo.newMadt5Entry(entry);
                break;
            }
        }
        i += entry_size - 3;
    }

    cPrint("\nCore count: %d\n", madtInfo.numOfmadt0Entries);
    for(uint64_t i = 0; i < madtInfo.numOfmadt0Entries; i++) {
        cPrint("Core %d\n", i + 1);
        cPrint("\tprocessor id: %d", madtInfo.madt0[i].ACPIprocessorID);
        cPrint("\tcore id: %d", madtInfo.madt0[i].APICid);
        cPrint("\tflags: %d\n", madtInfo.madt0[i].flags);
    }

    for(uint64_t i = 0; i < madtInfo.numOfmadt1Entries; i++) {
        cPrint("IOAPIC id: %d", madtInfo.madt1[i].ioapicID);
        cPrint("IOAPIC addr: %x", madtInfo.madt1[i].ioapicAddr);
        cPrint("GSI base %x\n", madtInfo.madt1[i].GSIBase);
    }

    for(uint64_t i = 0; i < madtInfo.numOfmadt2Entries; i++) {
        cPrint("ISO:\n");
        cPrint("\tbus: %d", madtInfo.madt2[i].busSource);
        cPrint("\tIRQ: %d", madtInfo.madt2[i].irqSource);
        cPrint("\tGSI: %d", madtInfo.madt2[i].GSI);
        cPrint("\tFlags: %x\n", madtInfo.madt2[i].flags);
    }

    for(uint64_t i = 0; i < madtInfo.numOfmadt4Entries; i++) {
        cPrint("ACPIprocessorID: %d", madtInfo.madt4[i].ACPIprocessorID);
        cPrint("flags: %x", madtInfo.madt4[i].flags);
        cPrint("lint: %d", madtInfo.madt4[i].lint); 
    }

    for(uint64_t i = 0; i < madtInfo.numOfmadt5Entries; i++) {
        cPrint("new lapic base address: %x", madtInfo.madt5[i].lapicBaseAddr);
    }
}

madtInfo_t::madtInfo_t()
{
    madt0 = (madt0_t*)malloc(sizeof(madt0_t) * 4);
    madt1 = (madt1_t*)malloc(sizeof(madt1_t));
    madt2 = (madt2_t*)malloc(sizeof(madt2_t) * 5);
    madt4 = (madt4_t*)malloc(sizeof(madt4_t));
    madt5 = (madt5_t*)malloc(sizeof(madt5_t));
}

void madtInfo_t::newMadt0Entry(madt0_t *entry)
{
    madt0_t newEntry(entry);
    static uint64_t maxMadt0 = 4;
    if(numOfmadt0Entries == maxMadt0) {
        madt0 = (madt0_t*)realloc(madt0, sizeof(madt0_t)); 
        maxMadt0 += 1;
    }

    madt0[numOfmadt0Entries++] = newEntry;
}

void madtInfo_t::newMadt1Entry(madt1_t *entry)
{
	madt1_t newEntry(entry);
    static uint64_t maxMadt1 = 1;
    if(numOfmadt1Entries == maxMadt1) {
        madt1 = (madt1_t*)realloc(madt1, sizeof(madt1_t));
        maxMadt1 += 1;
    }

	madt1[numOfmadt1Entries++] = newEntry;
}

void madtInfo_t::newMadt2Entry(madt2_t *entry)
{
	madt2_t newEntry(entry);
    static uint64_t maxMadt2 = 5;
    if(numOfmadt2Entries == maxMadt2) {
        madt2 = (madt2_t*)realloc(madt2, sizeof(madt2_t));
        maxMadt2 += 1;
    }

	madt2[numOfmadt2Entries++] = newEntry;
}

void madtInfo_t::newMadt4Entry(madt4_t *entry)
{
	madt4_t newEntry(entry);
    static uint64_t maxMadt4 = 5;
    if(numOfmadt4Entries == maxMadt4) {
        madt4 = (madt4_t*)realloc(madt0, sizeof(madt0_t));
        maxMadt4 += 1;
    }

	madt4[numOfmadt4Entries++] = entry;
}

void madtInfo_t::newMadt5Entry(madt5_t *entry)
{
	madt5_t newEntry(entry);
    static uint64_t maxMadt5 = 1;
    if(numOfmadt5Entries == maxMadt5) {
        madt5 = (madt5_t*)realloc(madt0, sizeof(madt5_t));
        maxMadt5 += 1;
    }

	madt5[numOfmadt5Entries++] = newEntry;
}

madt0_t::madt0_t(madt0_t *objPtr) 
{
	this->ACPIprocessorID = objPtr->ACPIprocessorID;
	this->APICid = objPtr->APICid;
	this->flags = objPtr->flags;
}

madt1_t::madt1_t(madt1_t *objPtr)
{
	this->ioapicID = objPtr->ioapicID;
	this->reserved = objPtr->reserved;
	this->ioapicAddr = objPtr->ioapicAddr;
	this->GSIBase = objPtr->GSIBase;
}

madt2_t::madt2_t(madt2_t *objPtr)
{
	this->busSource = objPtr->busSource;
	this->irqSource = objPtr->irqSource;
	this->GSI = objPtr->GSI;
	this->flags = objPtr->flags;
} 

madt4_t::madt4_t(madt4_t *objPtr)
{
	this->ACPIprocessorID = objPtr->ACPIprocessorID;
	this->flags = objPtr->flags;
	this->lint = objPtr->lint;
} 

madt5_t::madt5_t(madt5_t *objPtr)
{
	this->reserved = objPtr->reserved;
	this->lapicBaseAddr = objPtr->lapicBaseAddr;
}

madtInfo_t *getMadtInfo()
{
    return &madtInfo;
}
