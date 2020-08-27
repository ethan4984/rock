#include <kernel/fs/ext2/superblock.h>
#include <kernel/fs/ext2/ext2.h>
#include <kernel/drivers/ahci.h>
#include <lib/stringUtils.h>
#include <lib/output.h>

namespace kernel {

static void printInode(inode_t inode);
static void printBGD(blockGroupDescriptor_t bgd);

blockGroupDescriptor_t ext2_t::readBGD(uint64_t index) {
    blockGroupDescriptor_t bgd;

    static uint64_t bgdOffset = superblock.blockSize >= 2048 ? superblock.blockSize : superblock.blockSize * 2;

    uint64_t blockGroupIndex = (index - 1) / superblock.data.inodesPerGroup;

    ahci.read(&ahci.drives[0], partitions[0], bgdOffset + (sizeof(blockGroupDescriptor_t) * blockGroupIndex), sizeof(blockGroupDescriptor_t), &bgd);
    return bgd;
}

inode_t ext2_t::getInode(uint64_t index) {
    blockGroupDescriptor_t bgd = readBGD(index);

    printBGD(bgd);

    inode_t inode;

    uint64_t inodeTableIndex = (index - 1) % superblock.data.inodesPerGroup;
    
    ahci.read(&ahci.drives[0], partitions[0], (bgd.startingBlock * superblock.blockSize) + (superblock.data.inodeSize * inodeTableIndex), sizeof(inode_t), &inode);
    return inode;
}

void ext2_t::readInode(inode_t inode, uint64_t addr, uint64_t cnt, void *buffer) {
    uint32_t block = addr / superblock.blockSize;
    uint32_t blockOffset = addr % superblock.blockSize;

    if(block < 12) { // is direct block
        kprintDS("[KDEBUG]", "directly reading from block %d", inode.blocks[block]);
        ahci.read(&ahci.drives[0], partitions[0], inode.blocks[block] * superblock.blockSize + blockOffset, cnt, buffer);
        return;
    }

    if(block >= superblock.blockSize / 4) { // doubly indirect block
        block -= superblock.blockSize / 4;
        uint32_t doubleIndirectBlockIndex = block / (superblock.blockSize / 4);
        if(doubleIndirectBlockIndex >= superblock.blockSize / 4) { // triply indirect block
            return;
        }

        uint32_t indirectBlockIndex;
        uint32_t blockIndex;

        ahci.read(&ahci.drives[0], partitions[0], inode.blocks[13] * superblock.blockSize + doubleIndirectBlockIndex, sizeof(uint32_t), &indirectBlockIndex); // get the indirect block index
        ahci.read(&ahci.drives[0], partitions[0], indirectBlockIndex * superblock.blockSize, sizeof(uint32_t), &blockIndex); // get the block index
    
        kprintDS("[KDEBUG]", "doubly indirect reading from block %d", blockIndex);

        ahci.read(&ahci.drives[0], partitions[0], blockIndex * superblock.blockSize + blockOffset, cnt, buffer); // read that block
    } else { // singly indirect block
        uint32_t blockIndex;
        ahci.read(&ahci.drives[0], partitions[0], inode.blocks[12] * superblock.blockSize + block, sizeof(uint32_t), &blockIndex); // block index
        kprintDS("[KDEBUG]", "singly indirect reading from block %d", blockIndex);
        ahci.read(&ahci.drives[0], partitions[0], blockIndex * superblock.blockSize + blockOffset, cnt, buffer); // read that block
    }
}

static void printInode(inode_t inode) {
    kprintDS("[KDEBUG]", "permissions %x", inode.permissions);
    kprintDS("[KDEBUG]", "userID %x", inode.userID);
    kprintDS("[KDEBUG]", "size %x", inode.size32l);
    kprintDS("[KDEBUG]", "access time %x", inode.accessTime);
    kprintDS("[KDEBUG]", "creation time %x", inode.creationTime);
    kprintDS("[KDEBUG]", "modicication time %x", inode.modificationTime);
    kprintDS("[KDEBUG]", "deletion time %x", inode.deletionTime);
    kprintDS("[KDEBUG]", "group id %x", inode.groupID);
    kprintDS("[KDEBUG]", "hard link cnt %x", inode.hardLinkCnt);
    kprintDS("[KDEBUG]", "sector cnt %x", inode.sectorCnt);
    kprintDS("[KDEBUG]", "flags %x", inode.flags);
    kprintDS("[KDEBUG]", "oss1 %x", inode.oss1);
    kprintDS("[KDEBUG]", "generationNumber %x", inode.generationNumber);
    kprintDS("[KDEBUG]", "eab %x", inode.eab);
}

static void printBGD(blockGroupDescriptor_t bgd) {
    kprintDS("[KDEBUG]", "block address bitmap %x", bgd.blockAddressBitmap);
    kprintDS("[KDEBUG]", "blockAddressInodeBitmap %x", bgd.blockAddressInodeBitmap);
    kprintDS("[KDEBUG]", "startingBlock %x", bgd.startingBlock);
    kprintDS("[KDEBUG]", "unalloactedblocks %x", bgd.unallocatedBlocks);
    kprintDS("[KDEBUG]", "unallocatedInodes %x", bgd.unallocatedInodes);
    kprintDS("[KDEBUG]", "directory count %x", bgd.directoryCnt);
}

static void printDirEntry(directoryEntry_t dir) {
    kprintDS("[KDEBUG]", "inode: %d ", dir.inode);
    kprintDS("[KDEBUG]", "size: %d ", dir.sizeofEntry);
    kprintDS("[KDEBUG]", "name length: %d ", dir.nameLength);
    kprintDS("[KDEBUG]", "type: %d ", dir.typeIndicator);
}

void ext2_t::init() {
    superblock.read(0);

    if(superblock.data.magicNum != 0xef53) {
        cout + "[KDEBUG]" << "ext2 signature not found\n";        
        return;
    }
   
    kprintDS("[FS]", "parsing ext2 superblock");
    kprintDS("[FS]", "total number of inodes %d", superblock.data.inodeCount);
    kprintDS("[FS]", "total number of blocks %d", superblock.data.blockCount);
    kprintDS("[FS]", "superblock reserved block count %d", superblock.data.reservedBlocksCount);
    kprintDS("[FS]", "unalloacted blocks %d", superblock.data.freeBlocksCount);
    kprintDS("[FS]", "unalloacted inodes %d", superblock.data.freeInodesCount);
    kprintDS("[FS]", "block number containing the superblock %d", superblock.data.sbBlock);
    kprintDS("[FS]", "block size %d", superblock.blockSize);
    kprintDS("[FS]", "frafment size %d", 1024 << superblock.fragmentSize);
    kprintDS("[FS]", "blocks per block group %d", superblock.data.blocksPerGroup);
    kprintDS("[FS]", "fragments per group %d", superblock.data.fragsPerGroup);
    kprintDS("[FS]", "inodes per group %d", superblock.data.inodesPerGroup);
    kprintDS("[FS]", "filesystem state %d", superblock.data.state);
    kprintDS("[FS]", "Errors %d", superblock.data.errors);

    if(superblock.data.errors == 2) {
        cout + "[KDEBUG]" << "fatal ext2 errors\n";
        return;
    }

    rootInode = getInode(2);
    printInode(rootInode);
}

}
