#include <fs/ext2/block.h>

static uint32_t bgd_find_block(devfs_node_t *devfs_node, uint32_t bgd_index) {
    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;

    ext2_bgd_t bgd = ext2_read_bgd(devfs_node, bgd_index);
    uint8_t *bitmap = kcalloc(ext2->block_size);

    msd_raw_read(devfs_node, bgd.block_addr_bitmap, ext2->block_size, bitmap);
    for(uint32_t i = 0; i < ext2->block_size; i++) {
        if(!BM_TEST(bitmap, i)) {
            BM_SET(bitmap, i);
            msd_raw_write(devfs_node, bgd.block_addr_bitmap, ext2->block_size, bitmap);
            bgd.unallocated_blocks--;
            ext2_write_bgd(devfs_node, &bgd, bgd_index);
            kfree(bitmap);
            return i;
        }
    }
    kfree(bitmap);
    return -1;
}

uint32_t ext2_alloc_block(devfs_node_t *devfs_node) {
    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;

    for(uint32_t i = 0; i < ext2->bgd_cnt; i++) {
        ext2_bgd_t bgd = ext2_read_bgd(devfs_node, i);
        if(!bgd.unallocated_blocks)
            continue;

        uint32_t free_block = bgd_find_block(devfs_node, i);
        if(free_block == (uint32_t)-1)
            continue;

        return i * ext2->superblock.blocks_per_group + free_block;
    }

    return -1;
}

void ext2_free_block(devfs_node_t *devfs_node, uint32_t block) {
    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;
   
    uint8_t *bitmap = kcalloc(ext2->block_size);
    uint32_t bgd_index = block / ext2->superblock.blocks_per_group;
    uint32_t bitmap_index = block - (block / ext2->superblock.blocks_per_group);

    ext2_bgd_t bgd = ext2_read_bgd(devfs_node, bgd_index);

    msd_raw_read(devfs_node, bgd.block_addr_bitmap, ext2->block_size, bitmap);
    if(!BM_TEST(bitmap, bitmap_index)) {
        kfree(bitmap);
        return;
    }

    BM_CLEAR(bitmap, bitmap_index);
    msd_raw_write(devfs_node, bgd.block_addr_bitmap, ext2->block_size, bitmap);
    bgd.unallocated_blocks++;
    ext2_write_bgd(devfs_node, &bgd, bgd_index);
    kfree(bitmap);
}

ext2_bgd_t ext2_read_bgd(devfs_node_t *devfs_node, uint32_t index) {
    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;

    ext2_bgd_t bgd;

    size_t bgd_offset = ext2->block_size >= 2048 ? ext2->block_size : ext2->block_size * 2;
    size_t group_index = (index - 1) / ext2->superblock.inodes_per_group;

    msd_raw_read(devfs_node, bgd_offset + (sizeof(ext2_bgd_t) * group_index), sizeof(ext2_bgd_t), &bgd);

    return bgd;
}

void ext2_write_bgd(devfs_node_t *devfs_node, ext2_bgd_t *bgd, uint32_t index) {
    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;

    size_t bgd_offset = ext2->block_size >= 2048 ? ext2->block_size : ext2->block_size * 2;
    size_t group_index = (index - 1) / ext2->superblock.inodes_per_group;

    msd_raw_write(devfs_node, bgd_offset + (sizeof(ext2_bgd_t) * group_index), sizeof(ext2_bgd_t), bgd);
}
