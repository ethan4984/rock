#include <fs/ext2/ext2.hpp>

namespace ext2 {

fs::fs(dev::node &devfs_node) : devfs_node(devfs_node) {
    devfs_node.read(devfs_node.device->sector_size * 2, sizeof(superblock), &superb);

    if(superb.signature != 0xef53) {
        return;
    }

    print("[EXT2] Filesystem Detected on Device {}\n", devfs_node.vfs_node->absolute_path);

    block_size = 1024 << superb.block_size;
    frag_size = 1024 << superb.frag_size;
    bgd_cnt = superb.block_cnt / superb.blocks_per_group;
}

void fs::write_bgd(bgd *buf, uint32_t index) {
    size_t bgd_offset = block_size >= 2048 ? block_size : block_size * 2;
    size_t group_index = (index - 1) / superb.inodes_per_group;

    devfs_node.write(bgd_offset + sizeof(bgd) * group_index, sizeof(bgd), reinterpret_cast<void*>(buf));
}

bgd fs::read_bgd(uint32_t index) {
    size_t bgd_offset = block_size >= 2048 ? block_size : block_size * 2;
    size_t group_index = (index - 1) / superb.inodes_per_group;

    bgd ret;

    devfs_node.read(bgd_offset + sizeof(bgd) * group_index, sizeof(bgd), reinterpret_cast<void*>(&ret));

    return ret;
}

uint32_t fs::bgd_find_block(uint32_t bgd_index) {
    bgd bgd_cur = read_bgd(bgd_index);
    uint8_t *bitmap = new uint8_t[block_size];

    devfs_node.read(bgd_cur.block_addr_bitmap, block_size, reinterpret_cast<void*>(bitmap));

    for(size_t i = 0; i < block_size; i++) {
        if(!bm_test(bitmap, i)) {
            bm_set(bitmap, i);
            devfs_node.write(bgd_cur.block_addr_bitmap, block_size, reinterpret_cast<void*>(bitmap));
            bgd_cur.unallocated_blocks--;
            write_bgd(&bgd_cur, bgd_index);
            delete bitmap;
            return i;            
        }
    }

    delete bitmap;
    return -1;
}

uint32_t fs::alloc_block() {
    for(size_t i = 0; i < bgd_cnt; i++) {
        bgd bgd_cur = read_bgd(i);
        if(!bgd_cur.unallocated_blocks)
            continue;

        uint32_t free_block = bgd_find_block(i);
        if(free_block == ~0u)
            continue;

        return i * superb.blocks_per_group + free_block;
    }

    return -1;
}

void fs::free_block(uint32_t block) {
    uint8_t *bitmap = new uint8_t[block_size];
    uint32_t bgd_index = block / superb.blocks_per_group;
    uint32_t bitmap_index = block - bgd_index * superb.blocks_per_group;

    bgd bgd_cur = read_bgd(bgd_index);

    devfs_node.read(bgd_cur.block_addr_bitmap, block_size, reinterpret_cast<void*>(bitmap));
    if(!bm_test(bitmap, bitmap_index)) {
        delete bitmap;
        return;
    }

    bm_clear(bitmap, bitmap_index);
    devfs_node.write(bgd_cur.block_addr_bitmap, block_size, reinterpret_cast<void*>(bitmap));
    bgd_cur.unallocated_blocks++;
    write_bgd(&bgd_cur, bgd_index);
    delete bitmap;
}

uint32_t fs::alloc_inode() {
    uint8_t *bitmap = new uint8_t[block_size];
    for(size_t i = 0; i < bgd_cnt; i++) {
        bgd bgd_cur = read_bgd(i);
        if(!bgd_cur.unallocated_inodes)
            continue;

        devfs_node.read(bgd_cur.block_addr_inode, block_size, reinterpret_cast<void*>(bitmap));
        for(size_t j = 0; j < block_size; j++) {
            if(!bm_test(bitmap, j)) {
                bm_set(bitmap, j);
                devfs_node.write(bgd_cur.block_addr_inode, block_size, reinterpret_cast<void*>(bitmap));

                bgd_cur.unallocated_inodes--;
                write_bgd(&bgd_cur, i);

                delete bitmap;
                return i * superb.inodes_per_group + j;
            }
        }
    }

    delete bitmap;
    return -1;
}

void fs::free_inode(uint32_t index) {
    uint8_t *bitmap = new uint8_t[block_size];
    uint32_t bgd_index = index / superb.inodes_per_group;
    uint32_t bitmap_index = index - bgd_index * superb.inodes_per_group;

    bgd bgd_cur = read_bgd(bgd_index);

    devfs_node.read(bgd_cur.block_addr_inode, block_size, reinterpret_cast<void*>(bitmap));
    if(!bm_test(bitmap, bitmap_index)) {
        delete bitmap;
        return;
    }

    bm_clear(bitmap, bitmap_index);
    devfs_node.write(bgd_cur.block_addr_inode, block_size, reinterpret_cast<void*>(bitmap));

    bgd_cur.unallocated_inodes++;
    write_bgd(&bgd_cur, bgd_index);
}

void fs::inode_write_entry(inode *inode_cur, uint32_t index) {
    bgd bgd_cur = read_bgd(index);

    size_t inode_table_index = (index - 1) % superb.inodes_per_group;
    devfs_node.write(bgd_cur.inode_table_block * block_size + superb.inode_size * inode_table_index, sizeof(inode), reinterpret_cast<void*>(inode_cur));
}

inode fs::inode_read_entry(uint32_t index) {
    bgd bgd_cur = read_bgd(index);
   
    inode ret;

    size_t inode_table_index = (index - 1) % superb.inodes_per_group;
    devfs_node.read(bgd_cur.inode_table_block * block_size + superb.inode_size * inode_table_index, sizeof(inode), reinterpret_cast<void*>(&ret));

    return ret;
}

}
