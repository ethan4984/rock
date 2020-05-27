#pragma once

#include <stdint.h>
#include <Kernel/acpiUtils.h>

void madtParse();

class madt_t
{
    public:
        ACPIheader_t ACPIheader;
        uint32_t lapicAddr;
        uint32_t flags;
        uint8_t entries[];
} __attribute__((packed));

class madt0_t /* Processor Local APIC table */
{
    public:
        madt0_t(madt0_t *objPtr);

        uint8_t ACPIprocessorID;
        uint8_t APICid;
        uint32_t flags;

} __attribute__((packed));

class madt1_t /* I/O APIC */
{
    public:
		madt1_t(madt1_t *objPtr);
    
        uint8_t ioapicID;
        uint8_t reserved;
        uint32_t ioapicAddr;
        uint32_t GSIBase;
} __attribute__((packed));

class madt2_t /* ISO / Interrupt Source Override */
{
    public:
		madt2_t(madt2_t *objPtr);
		
        uint8_t busSource;
        uint8_t irqSource;
        uint32_t GSI;
        uint16_t flags;
} __attribute__((packed));

class madt4_t /* NMI / non-maskable interupts */
{
    public:
		madt4_t(madt4_t *objPtr);
		
        uint8_t ACPIprocessorID;
        uint16_t flags;
        uint8_t lint;
} __attribute__((packed));

class madt5_t /* lapic address override */
{
    public:
		madt5_t(madt5_t *objPtr);
		
        uint16_t reserved;
        uint64_t lapicBaseAddr;
} __attribute__((packed));

class madtInfo_t 
{
    public:
        madtInfo_t();

        uint64_t numOfmadt0Entries = 0;
        uint64_t numOfmadt1Entries = 0;
        uint64_t numOfmadt2Entries = 0;
        uint64_t numOfmadt4Entries = 0;
        uint64_t numOfmadt5Entries = 0;

        madt0_t *madt0;
        madt1_t *madt1;
        madt2_t *madt2;
        madt4_t *madt4;
        madt5_t *madt5;

        void newMadt0Entry(madt0_t *entry); 
        void newMadt1Entry(madt1_t *entry);
        void newMadt2Entry(madt2_t *entry);
        void newMadt4Entry(madt4_t *entry);
        void newMadt5Entry(madt5_t *entry);

        uint32_t lapicAddr;
} __attribute__((packed));

madtInfo_t *getMadtInfo();
