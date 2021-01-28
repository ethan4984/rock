#include <drivers/ahci.h>
#include <fs/device.h>
#include <bitmap.h>
#include <output.h>

static uint64_t ahci_drive_cnt = 0;
static ahci_drive_t *ahci_drives;

static void init_proto(pci_device_t *device);
static void init_sata_device(volatile port_regs_t *regs);
static void send_command(volatile port_regs_t *regs, uint32_t cmd_slot);
static void ahci_RW(ahci_drive_t *drive, uint64_t start, uint64_t cnt, void *buffer, uint8_t w); 
static int find_CMD(volatile port_regs_t *regs);

void ahci_init() {
    ahci_drives = kmalloc(sizeof(ahci_drive_t) * 32);

    for(uint64_t i = 0; i < pci_device_cnt; i++) {
        if((pci_devices[i].class_code == 1) && (pci_devices[i].sub_class == 6)) { 
            switch(pci_devices[i].prog_IF) {
                case 0:
                    kvprintf("[AHCI] detected a vendor specific interface (get a new pc)\n");
                    break;
                case 1:
                    kvprintf("[ACHI] detetced an AHCI 1.0 compatiable device\n");
                    init_proto(&pci_devices[i]);
                    break;
                case 2:
                    kvprintf("[AHCI] detetced a serial storage bus\n");
                    break;
            }
        }
    }
    
    for(uint64_t i = 0; i < ahci_drive_cnt; i++) {
        kvprintf("[AHCI] sata drive %d detected with %d sectors\n", i, ahci_drives[i].sector_cnt);
    }
}

static void init_proto(pci_device_t *device) {
    ahci_drives = kmalloc(sizeof(ahci_drive_t) * 32);

    pci_write(pci_read(device->bus, device->device, device->function, 0x4) | (1 << 2), device->bus, device->device, device->function, 0x4); // become a bus master

    pci_bar_t bar = pci_get_bar(*device, 5);
    volatile GHC_t *GHC = (volatile GHC_t*)((uint64_t)bar.base + HIGH_VMA);
    kvprintf("[AHCI] device's GHC found at %x\n", (uint64_t)GHC);

    for(uint8_t i = 0; i < 32; i++) {
        if(GHC->pi & (1 << i)) { // check for a valid port
            volatile port_regs_t *port_regs = (volatile port_regs_t*)&GHC->port[i];

            switch(port_regs->sig) {
                case SATA_ATA:
                    kvprintf("[AHCI] sata drive found on port %d\n", i);
                    init_sata_device(port_regs);
                    break; 
                case SATA_ATAPI:
                    kvprintf("[AHCI] enclosure management bridge found on port %d\n", i); 
                    break; 
                case SATA_PM:
                    kvprintf("[ACHI] port multipler found on port %d\n", i); 
            }
        }
    }
}

static int find_CMD(volatile port_regs_t *regs) {
    for(uint32_t i = 0; i < 32; i++) {
        if(((regs->sact | regs->ci) & (1 << i)) == 0)
            return i;
    }
    return -1; 
}

static void send_command(volatile port_regs_t *regs, uint32_t cmd_slot) {
    while((regs->tfd & (0x80 | 0x8)));

    regs->cmd &= ~HBA_CMD_ST;

    while(regs->cmd & HBA_CMD_CR);

    regs->cmd |= HBA_CMD_FRE;
    regs->cmd |= HBA_CMD_ST;
    
    regs->ci = 1 << cmd_slot;
    
    while(1) {
        if(!(regs->ci & (1 << cmd_slot)))
            break;
    }
    
    regs->cmd &= ~HBA_CMD_ST;
    while (regs->cmd & HBA_CMD_CR);
    regs->cmd &= ~HBA_CMD_FRE;
}

int ahci_read(int drive_index, uint64_t addr, uint64_t cnt, void *ret) {
    uint8_t *buffer = (uint8_t*)(pmm_alloc(ROUNDUP(cnt, 0x1000)) + HIGH_VMA);
    ahci_RW(&ahci_drives[drive_index], addr / 0x200, ROUNDUP(cnt, 0x200), buffer, 0);
    memcpy8(ret, buffer + (addr % 0x200), cnt);
    pmm_free((uint64_t)buffer - HIGH_VMA, ROUNDUP(cnt, 0x1000));
    return cnt;
}

int ahci_write(int drive_index, uint64_t addr, uint64_t cnt, void *buffer) {
    uint8_t *disk = (uint8_t*)(pmm_alloc(ROUNDUP(cnt, 0x1000)) + HIGH_VMA);
    ahci_RW(&ahci_drives[drive_index], addr / 0x200, ROUNDUP(cnt, 0x200), disk, 0);
    memcpy8(disk + (addr % 0x200), buffer, cnt);
    ahci_RW(&ahci_drives[drive_index], addr / 0x200, ROUNDUP(cnt, 0x200), disk, 1);
    pmm_free((uint64_t)disk - HIGH_VMA, ROUNDUP(cnt, 0x1000));
    return cnt;
}

static void init_sata_device(volatile port_regs_t *regs) {
    int cmd_slot = find_CMD(regs);
    if(cmd_slot == -1) {
        kprintf("[AHCI]", "Device command slots are full");
    }

    uint16_t *identity = (uint16_t*)(pmm_alloc(1) + HIGH_VMA);
    volatile hba_cmd_t *hba_cmd = (volatile hba_cmd_t*)((uint64_t)regs->clb + HIGH_VMA);

    hba_cmd += cmd_slot;
    hba_cmd->cfl = sizeof(fis_h2d_t) / sizeof(uint32_t);
    hba_cmd->w = 0;
    hba_cmd->prdtl = 1;

    volatile hba_command_table_t *cmdtbl = (volatile hba_command_table_t*)((uint64_t)hba_cmd->ctba + HIGH_VMA);
    memset8((uint8_t*)((uint64_t)hba_cmd->ctba + HIGH_VMA), 0, sizeof(volatile hba_command_table_t));

    cmdtbl->PRDT[0].dba = (uint32_t)(uint64_t)identity;
    cmdtbl->PRDT[0].dbc = 511;
    cmdtbl->PRDT[0].i = 1;

    fis_h2d_t *cmdfis = (fis_h2d_t*)(((uint64_t)cmdtbl->cfis));
    memset8((uint8_t*)(((uint64_t)cmdtbl->cfis)), 0, sizeof(fis_h2d_t));

    cmdfis->command = 0xec;
    cmdfis->c = 1;
    cmdfis->device = 0;
    cmdfis->pmport = 0;
    cmdfis->fisType = FIS_REG_H2D;

    send_command(regs, cmd_slot);

    ahci_drives[ahci_drive_cnt] = (ahci_drive_t) { *(uint64_t*)((uint64_t)&identity[100]), regs };

    msd_t device = { .device_index = ahci_drive_cnt++,
                     .read = ahci_read,
                     .write = ahci_write
                   };

    size_t vec_id = vec_push(msd_t, msd_list, device);

    scan_device_partitions(vec_search(msd_t, msd_list, vec_id));
}

static void ahci_RW(ahci_drive_t *drive, uint64_t start, uint64_t count, void *buffer, uint8_t w) {
    uint32_t cmd_slot = find_CMD(drive->regs);

    volatile hba_cmd_t *hba_cmd = (volatile hba_cmd_t*)((uint64_t)drive->regs->clb + HIGH_VMA);
    memset8((uint8_t*)((uint64_t)drive->regs->clb + HIGH_VMA), 0, sizeof(volatile hba_cmd_t));

    hba_cmd += cmd_slot;
    hba_cmd->cfl = sizeof(volatile fis_h2d_t) / sizeof(uint32_t);
    hba_cmd->w = (w) ? 1 : 0;
    hba_cmd->prdtl = 1;

    volatile hba_command_table_t *cmdtbl = (volatile hba_command_table_t*)((uint64_t)hba_cmd->ctba + HIGH_VMA);
    memset8((uint8_t*)((uint64_t)hba_cmd->ctba + HIGH_VMA), 0, sizeof(volatile hba_command_table_t));
    
    cmdtbl->PRDT[0].dba = (uint32_t)((uint64_t)buffer - HIGH_VMA);
    cmdtbl->PRDT[0].dbc = (count * 512) - 1;
    cmdtbl->PRDT[0].i = 1;
    
    fis_h2d_t *cmdfis = (fis_h2d_t*)(((uint64_t)cmdtbl->cfis));
    memset8((uint8_t*)(((uint64_t)cmdtbl->cfis)), 0, sizeof(fis_h2d_t));
    
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
    
    send_command(drive->
            regs, cmd_slot);
}
