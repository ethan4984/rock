#pragma once

#include <pci.h>
#include <stdint.h>

void ahci_init(pci_device_t *devices, pci_device_id_t *device_ids, uint64_t device_count);
