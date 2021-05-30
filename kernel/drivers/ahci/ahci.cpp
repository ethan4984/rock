#include <drivers/ahci/ahci.hpp>

namespace ahci {

device::device(pci::device pci_device) : pci_device(pci_device) {
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
}

}
