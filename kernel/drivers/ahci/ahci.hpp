#ifndef AHCI_HPP_
#define AHCI_HPP_

#include <drivers/pci.hpp>

namespace ahci {

constexpr size_t sata_ata = 0x101;
constexpr size_t sata_atapi = 0xeb140101;
constexpr size_t sata_semb = 0xc33C0101;
constexpr size_t sata_pm = 0x96690101;

constexpr size_t hba_cmd_st = 0x1;
constexpr size_t hba_cmd_fre = 0x10;
constexpr size_t hba_cmd_fr = 0x4000;
constexpr size_t hba_cmd_cr = 0x8000;

constexpr size_t fis_reg_h2d = 0x27;
constexpr size_t fis_reg_d2h = 0x34;
constexpr size_t fis_dma_enable = 0x39;
constexpr size_t fis_dma_setup = 0x41;
constexpr size_t fis_data = 0x46;
constexpr size_t fis_bist = 0x58;
constexpr size_t fis_pio_setup = 0x5f;
constexpr size_t fis_device_bits = 0xa1;

struct [[gnu::packed]] port_regs {
    uint32_t clb;
    uint32_t clbu;
    uint32_t fb;
    uint32_t fbu;
    uint32_t is;
    uint32_t ie;
    uint32_t cmd;
    uint32_t reserved0;
    uint32_t tfd;
    uint32_t sig;
    uint32_t ssts;
    uint32_t sstl;
    uint32_t serr;
    uint32_t sact;
    uint32_t ci;
    uint32_t sntf;
    uint32_t fbs;
    uint32_t devslp;
    uint32_t reserved1[11];
    uint32_t vs[10];
};

struct [[gnu::packed]] bar_regs {
    uint32_t cap;
    uint32_t ghc;
    uint32_t is; 
    uint32_t pi;
    uint32_t vs;
    uint32_t ccc_ctl;
    uint32_t ccc_ports;
    uint32_t em_lock;
    uint32_t em_ctl;
    uint32_t cap2;
    uint32_t bohc;
    uint32_t reserved[29];
    uint32_t vendor[24];
    volatile port_regs port[];
};

class device {
public:
    device(pci::device pci_device);
    size_t sector_cnt;
private:
    pci::device pci_device;
    pci::bar bar;
    volatile bar_regs regs;
};

inline lib::vector<device*> device_list;

}

#endif
