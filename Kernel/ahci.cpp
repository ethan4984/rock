#include <shitio.h>
#include <ahci.h>

using namespace standardout;

void ahci_init(pci_device_t *devices, pci_device_id_t *device_ids, uint64_t device_count)
{
    for(int i = 0; i < device_count; i++) {
        pci_device_id_t device_id = device_ids[i];
        if(device_id.class_code == 0x1 && device_id.subclass == 0x6) {
            k_print("ACHI init:\n\n");
            k_print("\tACHI: Serial ATA compatable device found\n\n");
            switch(device_id.prog_if) {
                case 0:
                    k_print("\tSerial ATA: [mode] Vendor Specific Interface\n");
                    break;
                case 1:
                    k_print("\tSerial ATA: [mode] ACHI 1.0\n");
                    break;
                case 2:
                    k_print("\tSerial ATA: [mode] Serial Storage Bus\n");
                    break;
                default:
                    k_print("\tSerial ATA: [mode] Undefined / Error\n");
                    break;
            }
            return;
        }
    }
    k_print("ACHI: Serial ATA compatable device not found\n\n");
}
