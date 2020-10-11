#include <kernel/fs/ext2/superblock.h>
#include <kernel/fs/ext2/inode.h>
#include <kernel/fs/ext2/ext2.h>
#include <kernel/drivers/ahci.h>
#include <kernel/fs/ext2/dir.h>
#include <kernel/mm/kHeap.h>
#include <lib/memoryUtils.h>
#include <lib/stringUtils.h>
#include <lib/output.h>

namespace ext2 {

static void printInode(inodeStruct_t inode);

BGD_t readBGD(uint64_t index, int partition) {
    BGD_t bgd;

    static uint64_t bgdOffset = superblock.blockSize >= 2048 ? superblock.blockSize : superblock.blockSize * 2;

    uint64_t blockGroupIndex = (index - 1) / superblock.data.inodesPerGroup;

    ahci::read(&ahci::drives[0], partitions[0], bgdOffset + (sizeof(BGD_t) * blockGroupIndex), sizeof(BGD_t), &bgd);
    return bgd;
}

size_t read(const char *path, uint64_t start, size_t cnt, void *buffer, int part) {
    dir dirEntry(path, part);

    inode_t inode(dirEntry.dirEntry.inode, part);

    inode.read(start, cnt, buffer, part);

    return cnt;
}

void init(int part) {
    superblock.read(0);

    if(superblock.data.magicNum != 0xef53) {
        cout + "[KDEBUG]" << "ext2 signature not found\n";        
        return;
    }
   
    kprintDS("[FS]", "parsing ext2 superblock");
    kprintDS("[FS]", "total number of inodes %d", superblock.data.inodeCount);
    kprintDS("[FS]", "total number of blocks %d", superblock.data.blockCount);
    kprintDS("[FS]", "superblock reserved block cnt %d", superblock.data.reservedBlocksCount);
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

    inode_t::rootInode = inode_t::getInode(2, part);
    printInode(inode_t::rootInode.inodeStruct);

    directory_t *dir = new directory_t;

    dir::getDir(inode_t::rootInode, dir, part);

    for(uint64_t i = 0; i < dir->dirCnt; i++) {
        kprintDS("[FS]", "%s", dir->names[i]);
    }

    delete dir;
}

static void printInode(inodeStruct_t inode) {
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

}
