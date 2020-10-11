#include <kernel/fs/ext2/superblock.h>
#include <kernel/fs/ext2/inode.h>
#include <kernel/fs/ext2/types.h>
#include <kernel/fs/ext2/ext2.h>
#include <kernel/drivers/ahci.h>

namespace ext2 {

inode_t inode_t::rootInode;

inode_t::inode_t(uint64_t index, int part) {
    *this = getInode(index, part);
}
	
inode_t inode_t::getInode(uint64_t index, int part) {
    BGD_t bgd = readBGD(index, part);

    inode_t inode;

    uint64_t inodeTableIndex = (index - 1) % superblock.data.inodesPerGroup;
    
    ahci::read(&ahci::drives[0], partitions[part], (bgd.startingBlock * superblock.blockSize) + (superblock.data.inodeSize * inodeTableIndex), sizeof(inode_t), &inode);
    return inode;
}

void inode_t::read(uint64_t start, uint64_t cnt, void *buffer, int part) {
    superblock.read(part);

    uint32_t headway = 0;

    while(headway < cnt) {
        uint64_t block = (start + headway) / superblock.blockSize;

        uint64_t size = cnt - headway;
        uint64_t offset = (start + headway) % superblock.blockSize;

        if (size > superblock.blockSize - offset)
            size = superblock.blockSize - offset;

        uint32_t blockIndex;

        if (block < 12) { // direct
            blockIndex = inodeStruct.blocks[block];
        } else { // indirect
            block -= 12;
            if (block * sizeof(uint32_t) >= superblock.blockSize) { // double indirect
                block -= superblock.blockSize / sizeof(uint32_t);
                uint32_t index  = block / (superblock.blockSize / sizeof(uint32_t));
                if (index * sizeof(uint32_t) >= superblock.blockSize) { // triple indirect
                    uint32_t doubleIndirect, indirectBlock, offset = block % (superblock.blockSize / sizeof(uint32_t));

                    ahci::read(&ahci::drives[0], partitions[part], inodeStruct.blocks[13] * superblock.blockSize + index * sizeof(uint32_t), sizeof(uint32_t), &doubleIndirect);
                    ahci::read(&ahci::drives[0], partitions[part], inodeStruct.blocks[13] * superblock.blockSize + index * sizeof(uint32_t), sizeof(uint32_t), &indirectBlock);
                    ahci::read(&ahci::drives[0], partitions[part], indirectBlock * superblock.blockSize + offset * sizeof(uint32_t), sizeof(uint32_t), &blockIndex);
                }
                uint32_t offset = block % (superblock.blockSize / sizeof(uint32_t));
                uint32_t indirect_block;

                ahci::read(&ahci::drives[0], partitions[part], inodeStruct.blocks[13] * superblock.blockSize + index * sizeof(uint32_t), sizeof(uint32_t), &indirect_block);
                ahci::read(&ahci::drives[0], partitions[part], indirect_block * superblock.blockSize + offset * sizeof(uint32_t), sizeof(uint32_t), &blockIndex);
            } else { // single indirect
                ahci::read(&ahci::drives[0], partitions[part], inodeStruct.blocks[12] * superblock.blockSize + block * sizeof(uint32_t), sizeof(uint32_t), &blockIndex);
            }
        }

        ahci::read(&ahci::drives[0], partitions[part], (blockIndex * superblock.blockSize) + offset, size, (void*)((uint64_t)buffer + headway));

        headway += size;
    }
}

}
