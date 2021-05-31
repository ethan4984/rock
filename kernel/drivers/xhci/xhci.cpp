#include <drivers/xhci/xhci.hpp>

namespace xhci {

controller::controller(pci::device pci_device) : pci_device(pci_device) {
    switch(pci_device.prog_if) {
        case 0x0:
            print("[USB] UHCI controller detcted\n");
            return;
        case 0x10:
            print("[USB] OHCI controller detcted\n");
            return;
        case 0x20:
            print("[USB] EHCI controller detected\n");
            return;
        case 0x30:
            print("[USB] XHCI controller detected\n");
            break;
        case 0x80:
            print("[USB] Controller type unspecified\n"); 
            return;
        case 0xfe:
            return;
    }

    pci_device.become_bus_master();
    pci_device.enable_mmio();
    pci_device.get_bar(bar, 0);

    cap = reinterpret_cast<volatile cap_regs*>(bar.base + vmm::high_vma);
    operation = reinterpret_cast<volatile operation_regs*>(bar.base + cap->cap_length + vmm::high_vma);
    runtime = reinterpret_cast<volatile runtime_regs*>(bar.base + (cap->runtime_reg_space_offset & ~0x1f) + vmm::high_vma);
    db = reinterpret_cast<volatile db_regs*>(bar.base + (cap->doorbell_offset & ~0x3) + vmm::high_vma);

    auto major_version = cap->interface_vs >> 8 & 0xff;
    auto minor_version = cap->interface_vs >> 4 & 0xf;
    auto tertiary_version = cap->interface_vs & 0xf;

    print("[XHCI] HCI Controller Detcted {}.{}.{}\n", major_version, minor_version, tertiary_version);
}

}
