#include <drivers/nvme.h>
#include <drivers/pci.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <output.h>

static void init_controller(pci_device_t *device);

void init_nvme() {
    for(uint64_t i = 0; i < pci_device_cnt; i++) { 
        if((pci_devices[i].class_code == 0x1) && (pci_devices[i].sub_class == 0x8)) {
            switch(pci_devices[i].prog_IF) {
                case 0x1:
                    kvprintf("[NVME] NVMHCI controller found\n");
                    break;
                case 0x2:
                    kvprintf("[NVME] NVM express controller found\n");
                    init_controller(&pci_devices[i]);
                    break;
                default: 
                    kvprintf("[NVME] Unknown controller found\n");
            }
        } 
    }
}

static void init_controller(pci_device_t *device) {
/*    pci_bar_t bar0 = pci_get_bar(*device, 0);

    volatile nvme_bar0_t *nvme_bar0 = (volatile nvme_bar0_t*)((uint64_t)bar0.base + HIGH_VMA); 
    pci_write(pci_read(device->bus, device->device, device->function, 0x4) | (1 << 2), device->bus, device->device, device->function, 0x4); // enable dma

    pci_write(pci_read(device->bus, device->device, device->function, 0x4) | (1 << 1), device->bus, device->device, device->function, 0x4); // enable mmio*/
}
