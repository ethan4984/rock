#ifndef E1000_H_
#define E1000_H_

#include <drivers/pci.h>
#include <vec.h>

#define E1000_REG_CTRL 0x0
#define E1000_REG_STATUS 0x8
#define E1000_REG_EECD 0x10
#define E1000_REG_EERD 0x14
#define E1000_REG_FLA 0x1c
#define E1000_REG_CTRL_EXIT 0x18
#define E1000_REG_MDIC 0x20
#define E1000_REG_FCAL 0x28
#define E1000_REG_FCAH 0x2c
#define E1000_REG_FCT 0x30
#define E1000_REG_VET 0x38
#define E1000_REG_FCTTV 0x170
#define E1000_REG_TXCW 0x178
#define E1000_REG_RXCW 0x180
#define E1000_REG_LEDCTL 0xe00
#define E1000_REG_PBA 0x1000
#define E1000_REG_ICR 0xc0
#define E1000_REG_ITR 0xc4
#define E1000_REG_ICS 0xc8
#define E1000_REG_IMS 0xd0

#define E1000_REG_RCTL 0x100
#define E1000_REG_FCRTL 0x2160
#define E1000_REG_FCRTH 0x2168
#define E1000_REG_RDBAL 0x2800
#define E1000_REG_RDBAH 0x2804
#define E1000_REG_RDLEN 0x2808
#define E1000_REG_RDH 0x2810
#define E1000_REG_RDL 0x2818
#define E1000_REG_RDTR 0x2820
#define E1000_REG_RADV 0x282c
#define E1000_REG_RSRPD 0x2c00

#define E1000_REG_TCTL 0x400
#define E1000_REG_TIPG 0x410
#define E1000_REG_AIFS 0x458
#define E1000_REG_TDBAL 0x3800
#define E1000_REG_TDBAH 0x3804
#define E1000_REG_TDLEN 0x3808
#define E1000_REG_TDH 0x3810
#define E1000_REG_TDL 0x3818
#define E1000_REG_TIDV 0x3820

#define E1000_RX_DESC_CNT 32
#define E1000_TX_DESC_CNT 8

#define E1000_RCTL_EN (1 << 1)
#define E1000_RCTL_SBP (1 << 2)
#define E1000_RCTL_UPE (1 << 3)
#define E1000_RCTL_MPE (1 << 4)
#define E1000_RCTL_LPE (1 << 5)
#define E1000_RCTL_LBM_NONE (0 << 6)
#define E1000_RCTL_LBM_PHY (3 << 6)
#define E1000_RTCL_RDMTS_HALF (0 << 8)
#define E1000_RTCL_RDMTS_QUARTER (1 << 8)
#define E1000_RTCL_RDMTS_EIGHTH (2 << 8)
#define E1000_RCTL_MO_36 (0 << 12)
#define E1000_RCTL_MO_35 (1 << 12)
#define E1000_RCTL_MO_34 (2 << 12)
#define E1000_RCTL_MO_32 (3 << 12)
#define E1000_RCTL_BAM (1 << 15)
#define E1000_RCTL_VFE (1 << 18)
#define E1000_RCTL_CFIEN (1 << 19)
#define E1000_RCTL_CFI (1 << 20)
#define E1000_RCTL_DPF (1 << 22)
#define E1000_RCTL_PMCF (1 << 23)
#define E1000_RCTL_SECRC (1 << 26)

#define E1000_RCTL_BSIZE_256 (3 << 16)
#define E1000_RCTL_BSIZE_512 (2 << 16)
#define E1000_RCTL_BSIZE_1024 (1 << 16)
#define E1000_RCTL_BSIZE_2048 (0 << 16)
#define E1000_RCTL_BSIZE_4096 ((3 << 16) | (1 << 25))
#define E1000_RCTL_BSIZE_8192 ((2 << 16) | (1 << 25))
#define E1000_RCTL_BSIZE_16384 ((1 << 16) | (1 << 25))

#define CMD_EOP                         (1 << 0)    // End of Packet
#define CMD_IFCS                        (1 << 1)    // Insert FCS
#define CMD_IC                          (1 << 2)    // Insert Checksum
#define CMD_RS                          (1 << 3)    // Report Status
#define CMD_RPS                         (1 << 4)    // Report Packet Sent
#define CMD_VLE                         (1 << 6)    // VLAN Packet Enable
#define CMD_IDE                         (1 << 7)    // Interrupt Delay Enable


struct e1000_rx_desc { 
    volatile uint64_t addr;
    volatile uint16_t length;
    volatile uint16_t checksum;
    volatile uint8_t status;
    volatile uint8_t errors;
    volatile uint16_t special;
} __attribute__((packed));

struct e1000_tx_desc { 
    volatile uint64_t addr;
    volatile uint16_t length;
    volatile uint8_t cso;
    volatile uint8_t cmd;
    volatile uint8_t status;
    volatile uint8_t css;
    volatile uint16_t special;
} __attribute__((packed));

struct nic;

struct e1000_nic {
    struct nic *parent;
    struct e1000_rx_desc *rx_desc[E1000_RX_DESC_CNT];
    struct e1000_tx_desc *tx_desc[E1000_TX_DESC_CNT];
    uint16_t current_rx;
    uint16_t current_tx;

    int (*send_packet)(struct nic*, const void *, uint16_t);
};

struct nic {
    const char *device_name;
    struct pci_device *device;
    struct pci_bar bar;
    uint8_t mac_address[6];
    struct e1000_nic *e1000;
};

uint32_t e1000_reg_read(struct nic *nic, uint32_t reg);
void e1000_reg_write(struct nic *nic, uint32_t reg, uint32_t data);
int e1000_send_packet(struct nic *nic, const void *data, uint16_t len);

void e1000_rx_init(struct nic *nic);
void e1000_tx_init(struct nic *nic);

void ethernet_init(struct pci_device *deivce);

extern_vec(struct nic, nic_list);

#endif
