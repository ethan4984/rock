#include <fs/ext2/ext2.hpp>

namespace ext2 {

fs::fs(dev::node &devfs_node) {
    devfs_node.read(devfs_node.device->sector_size * 2, sizeof(superblock), &superb);

    if(superb.signature != 0xef53) {
        return;
    }

    print("[EXT2] Filesystem Detected on Device {}\n", devfs_node.vfs_node->absolute_path);

    block_size = 1024 << superb.block_size;
    frag_size = 1024 << superb.frag_size;
    bgd_cnt = superb.block_cnt / superb.blocks_per_group;
}

}
