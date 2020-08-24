#include <kernel/fs/ext2/superblock.h>
#include <kernel/drivers/ahci.h>
#include <kernel/mm/kHeap.h>
#include <kernel/fs/vfs.h>
#include <lib/output.h>

namespace kernel {

void superblock_t::writeBack(uint8_t partitionIndex) {
    superBlockData_t *superBlockData = new superBlockData_t;
    *superBlockData = data;
    ahci.sataRW(&ahci.drives[0], partitions[partitionIndex].mbr.startingSector + 2, 1, superBlockData, 0);
    delete superBlockData;
}

void superblock_t::read(uint8_t partitionIndex) {
    superBlockData_t *superBlockData = new superBlockData_t;
    ahci.sataRW(&ahci.drives[0], partitions[partitionIndex].mbr.startingSector + 2, 1, superBlockData, 0);
    data = *superBlockData;
    blockSize = 1024 << data.blockSize;
    inodeSize = 1024 << data.inodeSize;
    delete superBlockData;
}

}
