#ifndef ACHI_H_
#define AHCI_H_

#define SATA_ATA 0x101
#define SATA_ATAPI 0xeb140101
#define SATA_SEMB 0xc33C0101
#define SATA_PM 0x96690101

#define HBA_CMD_ST 0x0001
#define HBA_CMD_FRE 0x0010
#define HBA_CMD_FR 0x4000
#define HBA_CMD_CR 0x8000

#define FIS_REG_H2D 0x27
#define FIS_REG_D2H 0x34
#define FIS_DMA_ENABLE 0x39
#define FIS_DMA_SETUP 0x41
#define FIS_DATA 0x46
#define FIS_BIST 0x58
#define FIS_PIO_SETUP 0x5f
#define FIS_DEVICE_BITS 0xa1

#include <drivers/pci.h>

typedef struct {
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
} port_regs_t;

typedef struct {
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
    volatile port_regs_t port[];
} GHC_t;

typedef struct {
    uint8_t cfl:5;
    uint8_t a:1;
    uint8_t w:1;
    uint8_t p:1;
    uint8_t r:1;
    uint8_t b:1;
    uint8_t c:1;
    uint8_t reserved0:1;
    uint8_t pmp:4;
    uint16_t prdtl;
    volatile uint32_t prdbc;
    uint32_t ctba;
    uint32_t ctbau;
    uint32_t rsv1[4];
} hba_cmd_t;

typedef struct {
    uint8_t fisType;

    uint8_t pmport:4;
    uint8_t reserved0:3;
    uint8_t c:1;

    uint8_t command;
    uint8_t featurel;

    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;

    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t featureh;

    uint8_t countl;
    uint8_t counth;
    uint8_t icc;
    uint8_t control;

    uint32_t reserved1;
} fis_h2d_t;

typedef struct {
    uint8_t fisType;
 
    uint8_t pmport:4;
    uint8_t reserved0:2;
    uint8_t i:1;
    uint8_t reserved1:1;

    uint8_t status;
    uint8_t error;

    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;

    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t reserved2;

    uint8_t countl;
    uint8_t counth;
    uint16_t reserved3;
    
    uint32_t reserved4;
} fis_d2h_t;

typedef struct {
    uint32_t dba;
    uint32_t dbau;
    uint32_t reserved0;
    uint32_t dbc:22;
    uint32_t reserved1:9;
    uint32_t i:1;
} hba_prdt_t;

typedef struct {
    uint8_t cfis[64];
    uint8_t acmd[16];
    uint8_t reserved[48];
    hba_prdt_t PRDT[1];   
} hba_command_table_t;

typedef struct {
    uint64_t sector_cnt;
    volatile port_regs_t *regs;
} ahci_drive_t;

void ahci_init();

int ahci_read(int drive_index, uint64_t start, uint64_t cnt, void *buf);

int ahci_write(int drive_index, uint64_t start, uint64_t cnt, void *buf);

#endif
