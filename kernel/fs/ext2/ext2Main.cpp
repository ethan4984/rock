#include <kernel/fs/ext2/superblock.h>
#include <kernel/fs/ext2/ext2Main.h>
#include <lib/output.h>

namespace kernel {

void ext2_t::init() {
    superblock.read();

    cout + "[FS]" << "inode count: " << superblock.data.inodeCount << "\n";
    cout + "[FS]" << "block count: " << superblock.data.blockCount << "\n";
    cout + "[FS]" << "free inodes: " << superblock.data.freeInodesCount << "\n";
    cout + "[FS]" << "free blocks: " << superblock.data.freeBlocksCount << "\n";
}

}
