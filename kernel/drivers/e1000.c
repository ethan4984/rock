#include <drivers/e1000.h>
#include <debug.h>

global_vec(nic_list);

uint32_t e1000_read_reg(struct nic *nic, uint32_t reg) {
    return *(volatile uint32_t*)(nic->bar.base + HIGH_VMA + reg);
}

void e1000_write_reg(struct nic *nic, uint32_t reg, uint32_t data) { 
    *(volatile uint32_t*)(nic->bar.base + HIGH_VMA + reg) = data;        
}

uint16_t e1000_eeprom_read(struct nic *nic, uint8_t addr) {
    uint32_t data;

    e1000_write_reg(nic, E1000_REG_EERD, 1 | ((uint32_t)addr << 8));
    while(!((data = e1000_read_reg(nic, E1000_REG_EERD)) & (1 << 4))); // pole the status of eerd.done

    return (uint16_t)(data >> 16);
}

void e1000_read_mac(struct nic *nic) {
    for(int i = 0, cnt = 0; i < 3; i++) {
        uint16_t data = e1000_eeprom_read(nic, i);
        nic->mac_address[cnt++] = (uint8_t)data;
        nic->mac_address[cnt++] = (uint8_t)(data >> 8);
    }
};

static void e1000_init(struct pci_device *device, const char *dev_name) {
    struct pci_bar bar;
    if(pci_get_bar(device, &bar, 0) == -1)
        return;

    struct nic nic = {  .device = device,
                        .bar = bar, 
                        .device_name = dev_name,
                        .e1000 = kmalloc(sizeof(struct e1000_nic))
                     };

    e1000_read_mac(&nic);

    kprintf("[%s] MAC address: %x:%x:%x:%x:%x:%x\n", dev_name, nic.mac_address[0],
                                                               nic.mac_address[1],
                                                               nic.mac_address[2],
                                                               nic.mac_address[3],
                                                               nic.mac_address[4],
                                                               nic.mac_address[5]);

    pci_become_bus_master(device); 

    vec_push(struct nic, nic_list, nic);
}

void ethernet_init(struct pci_device *device) {
    kprintf("[NET] ethernet controller found from vendor %x and dev id %x\n", device->vendor_id, device->device_id);
    if(device->vendor_id == 0x8086) {
        switch(device->device_id) {
            case 0x100e:
                kprintf("[NET] emmulated e1000 NICs detected\n");
                e1000_init(device, "E1000");
                break;
            case 0x153a:
                kprintf("[NET] I217 detected\n");
                e1000_init(device, "I217");
                break;
            case 0x10ea:
                kprintf("[NET] 82577LM detected\n");
                e1000_init(device, "82577LM");
                break;
            default:
                kprintf("[NET] unknown device from vendor %x and dev id %x\n", device->vendor_id, device->device_id);
        }
    }
}
