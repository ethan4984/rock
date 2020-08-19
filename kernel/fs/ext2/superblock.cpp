#include <kernel/fs/ext2/superblock.h>
#include <kernel/drivers/ahci.h>
#include <kernel/mm/kHeap.h>
#include <lib/output.h>

namespace kernel {

void superblock_t::writeBack() {
    superBlockData_t *superBlockData = new superBlockData_t;
    *superBlockData = data;
    ahci.sataRW(&ahci.drives[0], 2, 1, superBlockData, 0);
    delete superBlockData;
}

void superblock_t::read() {
    superBlockData_t *superBlockData = new superBlockData_t;
    ahci.sataRW(&ahci.drives[0], 2, 1, superBlockData, 0);
    data = *superBlockData;
    delete superBlockData;
}

}
