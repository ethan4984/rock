#include <drivers/ahci/ahci.hpp>

namespace ahci {

static lib::vector<device*> device_list;

controller::controller(pci::device pci_device) : pci_device(pci_device) {
    switch(pci_device.prog_if) {
        case 0:
            print("[AHCI] Detected a vendor specific interface (get a new pc)\n");
            return;
        case 1:
            print("[AHCI] Detected an AHCI 1.0 compatiable device\n");
            break;
        case 2:
            print("[AHCI] Detected a serial storage bus\n"); 
            return;
        default:
            print("[AHCI] Detected an unknown device type... aborting\n");
            return;
    }

    pci_device.become_bus_master();
    pci_device.enable_mmio();
    pci_device.get_bar(bar, 5);

    ghc_regs = reinterpret_cast<volatile bar_regs*>(bar.base + vmm::high_vma);

    if(!(ghc_regs->cap & (1 << 31))) {
        print("[AHCI] 64 bit addressing not supported\n");
        return;
    }

    port_cnt = ghc_regs->cap & 0b11111;
    cmd_slots = ghc_regs->cap >> 8 & 0b11111;

    auto major_version = ghc_regs->vs >> 16 & 0xffff;
    auto minor_version = ghc_regs->vs & 0xffff;

    print("[AHCI] Controller version {}.{}\n", major_version, minor_version);

    for(size_t i = 0; i < port_cnt; i++) {
        if(ghc_regs->pi & (1 << i)) {
            volatile port_regs *regs = reinterpret_cast<volatile port_regs*>(&ghc_regs->port[i]);

            switch(regs->sig) {
                case sata_ata: {
                    print("[AHCI] Sata Drive found on Port {}\n", i);

                    device *new_device = new device(this, regs); 
                    device_list.push(new_device);

                    break;
                }
                case sata_atapi: {
                    print("[AHCI] Enclosure management bridge found on port {}\n", i);
                    break;
                }
                case sata_pm: {
                    print("[AHCI] Port Multipler found on port {}\n", i);
                    break;
                }
            }
        }
    }
}

device::device(controller *parent, volatile port_regs *regs) : parent(parent), regs(regs) {
    auto cmd_slot = find_cmd_slot();
    if(cmd_slot == -1) {
        print("[AHCI] Unable to find free cmd slot... aborting\n");
        return;
    }

    volatile hba_cmd *cmd_hdr = [&]() {
        volatile hba_cmd *cmd_hdr = reinterpret_cast<volatile hba_cmd*>(regs->clb + vmm::high_vma) + cmd_slot;

        cmd_hdr->cfl = sizeof(fis_h2d) / sizeof(uint32_t);
        cmd_hdr->w = 0;
        cmd_hdr->prdtl = 1;

        return cmd_hdr;
    } ();

    uint16_t *identity = reinterpret_cast<uint16_t*>(pmm::alloc(1) + vmm::high_vma);

    volatile hba_command_table *cmd_table = [&]() {
        volatile hba_command_table *cmd_table = reinterpret_cast<volatile hba_command_table*>(cmd_hdr->ctba + vmm::high_vma);
        memset8(reinterpret_cast<uint8_t*>(cmd_hdr->ctba + vmm::high_vma), 0, sizeof(hba_command_table));

        cmd_table->PRDT[0].dba = reinterpret_cast<uint64_t>(identity) & 0xffffffff;
        cmd_table->PRDT[0].dbc = 511;
        cmd_table->PRDT[0].i = 1;

        return cmd_table;
    } ();

    fis_h2d *cmd = reinterpret_cast<fis_h2d*>(reinterpret_cast<size_t>(cmd_table->cfis));
    memset8(reinterpret_cast<uint8_t*>(cmd), 0, sizeof(fis_h2d));

    cmd->command = 0xec;
    cmd->c = 1;
    cmd->device = 0;
    cmd->pmport = 0;
    cmd->fis_type = fis_reg_h2d;

    send_cmd(cmd_slot);

    sector_cnt = *reinterpret_cast<size_t*>(&identity[100]);

    print("[AHCI] SATA drive detected with {x} sectors\n", sector_cnt);
}

int device::find_cmd_slot() {
    for(size_t i = 0; i < parent->cmd_slots; i++) {
        if(((regs->sact | regs->ci) & (1 << i)) == 0)
            return i;
    }
   
    return -1;
}

void device::send_cmd(size_t slot) {
    while((regs->tfd & (0x80 | 0x8)));

    regs->cmd = regs->cmd & ~hba_cmd_st;

    while(regs->cmd & hba_cmd_cr);

    regs->cmd = regs->cmd | hba_cmd_fre | hba_cmd_st;
    regs->ci = 1 << slot;

    for(;;) {
        if(!(regs->ci & (1 << slot)))
            break;
    }

    regs->cmd = regs->cmd & ~hba_cmd_st;
    while(regs->cmd & hba_cmd_st);
    regs->cmd = regs->cmd & ~hba_cmd_fre;
}

void device::lba_rw(size_t start, size_t cnt, void *buffer, bool w) {
    int cmd_slot = find_cmd_slot();
    if(cmd_slot == -1) {
        return;
    }

    volatile hba_cmd *cmd_hdr = [&]() {
        volatile hba_cmd *cmd_hdr = reinterpret_cast<volatile hba_cmd*>(regs->clb + vmm::high_vma) + cmd_slot;

        cmd_hdr->cfl = sizeof(fis_h2d) / sizeof(uint32_t);
        cmd_hdr->w = 0;
        cmd_hdr->prdtl = 1;

        return cmd_hdr;
    } ();

    volatile hba_command_table *cmd_table = [&]() {
        volatile hba_command_table *cmd_table = reinterpret_cast<volatile hba_command_table*>(cmd_hdr->ctba + vmm::high_vma);
        memset8(reinterpret_cast<uint8_t*>(cmd_hdr->ctba + vmm::high_vma), 0, sizeof(hba_command_table));

        cmd_table->PRDT[0].dba = reinterpret_cast<uint64_t>(buffer) - vmm::high_vma;
        cmd_table->PRDT[0].dbc = (cnt * sector_size) - 1;
        cmd_table->PRDT[0].i = 1;

        return cmd_table;
    } ();

    fis_h2d *cmd = reinterpret_cast<fis_h2d*>(reinterpret_cast<size_t>(cmd_table->cfis));
    memset8(reinterpret_cast<uint8_t*>(cmd), 0, sizeof(fis_h2d));

    cmd->fis_type = fis_reg_h2d;
    cmd->c = 1;
    cmd->command = w ? 0x35 : 0x25;
    cmd->device = 1 << 6; // LBA mode

    cmd->lba0 = start & 0xff;
    cmd->lba1 = start >> 8 & 0xff;
    cmd->lba2 = start >> 16 & 0xff;
    cmd->lba3 = start >> 24 & 0xff;
    cmd->lba4 = start >> 32 & 0xff;
    cmd->lba5 = start >> 40 & 0xff;

    send_cmd(cmd_slot);
}

}
