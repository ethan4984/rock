#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <kernel/drivers/ahci.h>
#include <kernel/drivers/pci.h>
#include <lib/memoryUtils.h>
#include <lib/output.h>

namespace kernel {

static pciBar_t bar;
    
void ahci_t::initAHCI() {
    pciInfo_t device;

    for(uint64_t i = 0; i < pci.totalDevices; i++) {
        if((pci.pciDevice[i].classCode == 1) && (pci.pciDevice[i].subclass == 6)) {
            switch(pci.pciDevice[i].progIF) {
                case 0:
                    cout + "[AHCI]" << "Detected a Vendor Sepcific Interface (get a new pc retard)\n";
                    return;
                case 1:
                    cout + "[AHCI]" << "Detected an AHCI 1.0 device\n";
                    device = pci.pciDevice[i];
    
                    if(!(pci.pciRead(device.bus, device.device, device.function, 0x4) & (1 << 2))) {
                        pci.pciWrite(pci.pciRead(device.bus, device.device, device.function, 0x4) | (1 << 2), device.bus, device.device, device.function, 0x4);
                    }

                    break;
                case 2:
                    cout + "[AHCI]" << "Detected a Serial Storage Bus\n";
                    return;
            }
        }
    }
   
    drives = new drive_t[32];

    bar = pci.getBAR(device, 5);

    cout + "[AHCI]" << "BAR5 base " << bar.base << " | size " <<  bar.size << "\n";
    
    volatile GHC_t *GHC = (volatile GHC_t*)((uint64_t)bar.base + HIGH_VMA - 0x1000);

    for(uint64_t i = 0; i < 32; i++) {
        if(GHC->pi & (1 << i)) {
            volatile hbaPorts_t *hbaPorts = &GHC->hbaPorts[i];

            switch(hbaPorts->sig) {
                case SATA_ATA:
                    //kprintDS("[AHCI]", "Sata drive found on port %d", i);
                    cout + "[AHCI]" << "Sata drive found on port " << i << "\n";
                    initSATAdevice(hbaPorts);
                    break;
                case SATA_ATAPI:
                    cout + "[AHCI]" << "ATAPI drive found on port " << i << "\n";
                    break;
                case SATA_SEMB:
                    cout + "[AHCI]" << "Enclosure management bridge found on port " << i << "\n";
                    break; 
                case SATA_PM:
                    cout + "[AHCI]" << "Port multipler found on port " << i << "\n";
            }
        }
    }
}

void ahci_t::sendCommand(volatile hbaPorts_t *hbaPort, uint32_t CMDslot) {
    while((hbaPort->tfd & (0x80 | 0x8)));

    hbaPort->cmd &= ~HBA_CMD_ST;

    while(hbaPort->cmd & HBA_CMD_CR);

    hbaPort->cmd |= HBA_CMD_FRE;
    hbaPort->cmd |= HBA_CMD_ST;
    
    hbaPort->ci = 1 << CMDslot;
    
    while(1) {
        if(!(hbaPort->ci & (1 << CMDslot)))
            break;
    }
    
    hbaPort->cmd &= ~HBA_CMD_ST;
    while (hbaPort->cmd & HBA_CMD_CR);
    hbaPort->cmd &= ~HBA_CMD_FRE;
}

uint32_t ahci_t::findCMD(volatile hbaPorts_t *hbaPort) {
    for(int i = 0; i < 32; i++) {
        if(((hbaPort->sact | hbaPort->ci) & (1 << i)) == 0)
            return i;
    }
    return 0;
}

void ahci_t::initSATAdevice(volatile hbaPorts_t *hbaPort) {
    static uint64_t driveCnt = 0;

    uint32_t CMDslot = findCMD(hbaPort);
    uint16_t *identify = (uint16_t*)physicalPageManager.alloc(1);
    volatile hbaCMDhdr_t *hbaCMDhdr = (volatile hbaCMDhdr_t*)((uint64_t)hbaPort->clb + HIGH_VMA);

    hbaCMDhdr += CMDslot;
    hbaCMDhdr->cfl = sizeof(volatile fisH2D_t) / sizeof(uint32_t);
    hbaCMDhdr->w = 0;
    hbaCMDhdr->prdtl = 1;

    volatile hbaCommandTable_t *cmdtbl = (volatile hbaCommandTable_t*)((uint64_t)hbaCMDhdr->ctba + HIGH_VMA);
    memset((void*)((uint64_t)hbaCMDhdr->ctba + HIGH_VMA), 0, sizeof(volatile hbaCommandTable_t));

    cmdtbl->PRDT[0].dba = (uint32_t)(uint64_t)identify;
    cmdtbl->PRDT[0].dbc = 511;
    cmdtbl->PRDT[0].i = 1;

    fisH2D_t *cmdfis = (fisH2D_t*)(((uint64_t)cmdtbl->cfis));
    memset((void*)(((uint64_t)cmdtbl->cfis)), 0, sizeof(fisH2D_t));

    cmdfis->command = 0xec;
    cmdfis->c = 1;
    cmdfis->device = 0;
    cmdfis->pmport = 0;
    cmdfis->fisType = FIS_REG_H2D;

    sendCommand(hbaPort, CMDslot);
    
    cout + "[AHCI]" << "Drive " << driveCnt++ << ": Total sector count: " << *((uint64_t*)((uint64_t)&identify[100] + HIGH_VMA)) << "\n";
    drives[driveCnt - 1] = (drive_t) { *((uint64_t*)((uint64_t)&identify[100] + HIGH_VMA)), hbaPort };
}

void ahci_t::sataRW(drive_t *drive, uint64_t start, uint32_t count, void *buffer, bool w) {
    uint32_t CMDslot = findCMD(drive->hbaPort);

    volatile hbaCMDhdr_t *hbaCMDhdr = (volatile hbaCMDhdr_t*)((uint64_t)drive->hbaPort->clb + HIGH_VMA);
    memset((void*)((uint64_t)drive->hbaPort->clb + HIGH_VMA), 0, sizeof(volatile hbaCMDhdr_t));

    hbaCMDhdr += CMDslot;
    hbaCMDhdr->cfl = sizeof(volatile fisH2D_t) / sizeof(uint32_t);
    hbaCMDhdr->w = (w) ? 1 : 0;
    hbaCMDhdr->prdtl = 1;
    
    volatile hbaCommandTable_t *cmdtbl = (volatile hbaCommandTable_t*)((uint64_t)hbaCMDhdr->ctba + HIGH_VMA);
    memset((void*)((uint64_t)hbaCMDhdr->ctba + HIGH_VMA), 0, sizeof(volatile hbaCommandTable_t));

    cmdtbl->PRDT[0].dba = (uint32_t)((uint64_t)buffer - HIGH_VMA);
    cmdtbl->PRDT[0].dbc = (count * 512) - 1;
    cmdtbl->PRDT[0].i = 1;
    
    fisH2D_t *cmdfis = (fisH2D_t*)(((uint64_t)cmdtbl->cfis));
    memset((void*)(((uint64_t)cmdtbl->cfis)), 0, sizeof(fisH2D_t));
    
    cmdfis->fisType = FIS_REG_H2D;
    cmdfis->c = 1;
    cmdfis->command = (w) ? 0x35 : 0x25;
    
    cmdfis->lba0 = (uint8_t)((uint32_t)start & 0x000000000000ff);
    cmdfis->lba1 = (uint8_t)(((uint32_t)start & 0x0000000000ff00) >> 8);
    cmdfis->lba2 = (uint8_t)(((uint32_t)start & 0x00000000ff0000) >> 16);
    
    cmdfis->device = 1 << 6;
    
    cmdfis->lba3 = (uint8_t)(((uint32_t)start & 0x000000ff000000) >> 24);
    cmdfis->lba4 = (uint8_t)(((start >> 32) & 0x0000ff00000000));
    cmdfis->lba5 = (uint8_t)(((start >> 32) & 0x00ff0000000000) >> 8);
    
    cmdfis->countl = (uint8_t)count;
    cmdfis->counth = (uint8_t)(count >> 8);
    
    sendCommand(drive->hbaPort, CMDslot);
}

}

