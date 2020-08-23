#include <kernel/fs/ext2/superblock.h>
#include <kernel/fs/ext2/ext2Main.h>
#include <lib/output.h>

namespace kernel {

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
    kprintDS("[FS]", "block size %d", 1024 << superblock.data.blockSize);
    kprintDS("[FS]", "frafment size %d", 1024 << superblock.data.fragSize);
    kprintDS("[FS]", "blocks per block group %d", superblock.data.blocksPerGroup);
    kprintDS("[FS]", "fragments per group %d", superblock.data.fragsPerGroup);
    kprintDS("[FS]", "inodes per group %d", superblock.data.inodesPerGroup);
    kprintDS("[FS]", "filesystem state %d", superblock.data.state);
    kprintDS("[FS]", "Errors %d", superblock.data.errors);

    if(superblock.data.errors == 2) {
        cout + "[KDEBUG]" << "fatal ext2 errors\n";
        return;
    }
}

}
