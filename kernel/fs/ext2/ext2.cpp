#include <kernel/fs/ext2/superblock.h>
#include <kernel/fs/ext2/ext2.h>
#include <kernel/drivers/ahci.h>
#include <kernel/mm/kHeap.h>
#include <lib/memoryUtils.h>
#include <lib/stringUtils.h>
#include <lib/output.h>

namespace kernel {

static void printInode(inode_t inode);
static void printBGD(blockGroupDescriptor_t bgd);
static void printDirEntry(directoryEntry_t dir);

blockGroupDescriptor_t ext2_t::readBGD(uint64_t index) {
    blockGroupDescriptor_t bgd;

    static uint64_t bgdOffset = superblock.blockSize >= 2048 ? superblock.blockSize : superblock.blockSize * 2;

    uint64_t blockGroupIndex = (index - 1) / superblock.data.inodesPerGroup;

    ahci.read(&ahci.drives[0], partitions[0], bgdOffset + (sizeof(blockGroupDescriptor_t) * blockGroupIndex), sizeof(blockGroupDescriptor_t), &bgd);
    return bgd;
}

inode_t ext2_t::getInode(uint64_t index) {
    blockGroupDescriptor_t bgd = readBGD(index);

    inode_t inode;

    uint64_t inodeTableIndex = (index - 1) % superblock.data.inodesPerGroup;
    
    ahci.read(&ahci.drives[0], partitions[0], (bgd.startingBlock * superblock.blockSize) + (superblock.data.inodeSize * inodeTableIndex), sizeof(inode_t), &inode);
    return inode;
}

directoryEntry_t ext2_t::getDirEntry(inode_t inode, const char *path) {
    char *buffer = new char[0x400];
    readInode(inode, 0, 0x400, buffer);

    directoryEntry_t *dir = new directoryEntry_t;
    directoryEntry_t ret;

    char **paths = new char*[256];

    uint64_t cnt = splitString(paths, path, "/");

    for(uint64_t j = 0; j < cnt; j++) {
        for(uint32_t i = 0; i < rootInode.size32l; i++) {     
            dir = (directoryEntry_t*)((uint64_t)buffer + i);

            if(strncmp(dir->name, paths[j], strlen(paths[j]) - 1) == 0 && j == cnt - 1) {
                ret = *dir;
                goto end;
            }

            if(strncmp(dir->name, paths[j], strlen(paths[j]) - 1) == 0) {
                inode = getInode(dir->inode);
                if(!(inode.permissions & 0x4000)) {
                    kprintDS("[KDEBUG]", "%s is not a directory", paths[j]); 
                }
                readInode(inode, 0, 0x400, buffer);
                continue;
            }

            if(dir->sizeofEntry != 0)
                i += dir->sizeofEntry - 1;
        }
    }
    
    kprintDS("[KDEBUG]", "%s not found", path);

end: // todo: get smart pointers setup so we dont have to deal with this mess
    for(uint64_t i = 0; i < cnt; i++)
        delete paths[i];
    delete paths;
    delete buffer;
    delete dir;
    return ret; 
}

void ext2_t::readInode(inode_t inode, uint64_t start, uint64_t cnt, void *buffer) {
    superblock.read(0);

    uint32_t headway = 0;

    while(headway < cnt) {
        uint64_t block = (start + headway) / superblock.blockSize;

        uint64_t size = cnt - headway;
        uint64_t offset = (start + headway) % superblock.blockSize;

        if (size > superblock.blockSize - offset)
            size = superblock.blockSize - offset;

        uint32_t blockIndex;

        if (block < 12) { // direct
            blockIndex = inode.blocks[block];
        } else { // indirect
            block -= 12;
            if (block * sizeof(uint32_t) >= superblock.blockSize) { // double indirect
                block -= superblock.blockSize / sizeof(uint32_t);
                uint32_t index  = block / (superblock.blockSize / sizeof(uint32_t));
                if (index * sizeof(uint32_t) >= superblock.blockSize) { // triple indirect
                    uint32_t doubleIndirect, indirectBlock, offset = block % (superblock.blockSize / sizeof(uint32_t));

                    ahci.read(&ahci.drives[0], partitions[0], inode.blocks[13] * superblock.blockSize + index * sizeof(uint32_t), sizeof(uint32_t), &doubleIndirect);
                    ahci.read(&ahci.drives[0], partitions[0], inode.blocks[13] * superblock.blockSize + index * sizeof(uint32_t), sizeof(uint32_t), &indirectBlock);
                    ahci.read(&ahci.drives[0], partitions[0], indirectBlock * superblock.blockSize + offset * sizeof(uint32_t), sizeof(uint32_t), &blockIndex);
                }
                uint32_t offset = block % (superblock.blockSize / sizeof(uint32_t));
                uint32_t indirect_block;

                ahci.read(&ahci.drives[0], partitions[0], inode.blocks[13] * superblock.blockSize + index * sizeof(uint32_t), sizeof(uint32_t), &indirect_block);
                ahci.read(&ahci.drives[0], partitions[0], indirect_block * superblock.blockSize + offset * sizeof(uint32_t), sizeof(uint32_t), &blockIndex);
            } else { // single indirect
                ahci.read(&ahci.drives[0], partitions[0], inode.blocks[12] * superblock.blockSize + block * sizeof(uint32_t), sizeof(uint32_t), &blockIndex);
            }
        }

        ahci.read(&ahci.drives[0], partitions[0], (blockIndex * superblock.blockSize) + offset, size, (void*)((uint64_t)buffer + headway));

        headway += size;
    }
}

void ext2_t::getDir(inode_t *inode, directory_t *ret) {
    directoryEntry_t *dir = new directoryEntry_t;

    char *buffer = new char[0x400];
    readInode(*inode, 0, 0x400, buffer);

    char **names = new char*[256];

    directoryEntry_t *dirBuffer = new directoryEntry_t[10];

    uint64_t cnt = 0;

    for(uint32_t i = 0; i < inode->size32l;) {
        dir = (directoryEntry_t*)((uint64_t)buffer + i);

        dirBuffer[cnt] = *dir;
    
        names[cnt] = new char[dir->nameLength];
        strncpy(names[cnt], dir->name, dir->nameLength);

        names[cnt][dir->nameLength] = '\0';

        i += dir->sizeofEntry;
        cnt++;
    }

    delete buffer;

    *ret = (directory_t) { dirBuffer, names, cnt }; // its up to the caller to free dirBuffer/names when theyre done with it
}

void ext2_t::read(const char *path, uint64_t start, uint64_t cnt, void *buffer) {
    directoryEntry_t dirEntry = getDirEntry(rootInode, path);     

    inode_t inode = getInode(dirEntry.inode);

    readInode(inode, start, cnt, buffer);
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

    rootInode = getInode(2);
    printInode(rootInode);

    directory_t *dir = new directory_t;

    getDir(&rootInode, dir);

    for(uint64_t i = 0; i < dir->dirCnt; i++) {
        kprintDS("[FS]", "%s", dir->names[i]);
    }

    delete dir;
}

__attribute__((unused))
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

__attribute__((unused))
static void printBGD(blockGroupDescriptor_t bgd) {
    kprintDS("[KDEBUG]", "block address bitmap %x", bgd.blockAddressBitmap);
    kprintDS("[KDEBUG]", "blockAddressInodeBitmap %x", bgd.blockAddressInodeBitmap);
    kprintDS("[KDEBUG]", "startingBlock %x", bgd.startingBlock);
    kprintDS("[KDEBUG]", "unalloactedblocks %x", bgd.unallocatedBlocks);
    kprintDS("[KDEBUG]", "unallocatedInodes %x", bgd.unallocatedInodes);
    kprintDS("[KDEBUG]", "directory cnt %x", bgd.directoryCnt);
}

__attribute__((unused))
static void printDirEntry(directoryEntry_t dir) {
    kprintDS("[KDEBUG]", "inode: %d ", dir.inode);
    kprintDS("[KDEBUG]", "size: %d ", dir.sizeofEntry);
    kprintDS("[KDEBUG]", "name length: %d ", dir.nameLength);
    kprintDS("[KDEBUG]", "type: %d ", dir.typeIndicator);
}

}
