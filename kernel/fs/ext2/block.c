#include <fs/ext2/block.h>
#include <bitmap.h>

void ext2_free_block(partition_t *part, uint32_t block) {
    uint8_t *bitmap = kcalloc(part->ext2_fs->block_size);
    uint32_t bgd_index = block / part->ext2_fs->superblock.blocks_per_group; 
    uint32_t bitmap_index = block - (block / part->ext2_fs->superblock.blocks_per_group);

    ext2_bgd_t bgd = ext2_read_bgd(part, bgd_index);

    partition_read(part, bgd.block_addr_bitmap, part->ext2_fs->block_size, bitmap);
    if(!BM_TEST(bitmap, bitmap_index)) {
        kfree(bitmap);
        return;
    }

    BM_CLEAR(bitmap, bitmap_index);
    partition_write(part, bgd.block_addr_bitmap, part->ext2_fs->block_size, bitmap);
    bgd.unallocated_blocks++;
    ext2_write_bgd(part, bgd, bgd_index);
    kfree(bitmap);
}

void ext2_alloc_block(partition_t *part, uint32_t block) {
    uint8_t *bitmap = kcalloc(part->ext2_fs->block_size);
    uint32_t bgd_index = block / part->ext2_fs->superblock.blocks_per_group; 
    uint32_t bitmap_index = block - (block / part->ext2_fs->superblock.blocks_per_group);

    ext2_bgd_t bgd = ext2_read_bgd(part, bgd_index);

    partition_read(part, bgd.block_addr_bitmap, part->ext2_fs->block_size, bitmap);
    if(BM_TEST(bitmap, bitmap_index)) {
        kfree(bitmap);
        return;
    }

    BM_SET(bitmap, bitmap_index);
    partition_write(part, bgd.block_addr_bitmap, part->ext2_fs->block_size, bitmap);
    bgd.unallocated_blocks++;
    ext2_write_bgd(part, bgd, bgd_index);
    kfree(bitmap);
}

ext2_bgd_t ext2_read_bgd(partition_t *part, uint32_t index) {
    ext2_bgd_t bgd;

    uint64_t bgd_offset = part->ext2_fs->block_size >= 2048 ? part->ext2_fs->block_size : part->ext2_fs->block_size * 2;
    uint64_t group_index = (index - 1) / part->ext2_fs->superblock.inodes_per_group;

    partition_read(part, bgd_offset + (sizeof(ext2_bgd_t) * group_index), sizeof(ext2_bgd_t), &bgd);
    return bgd;
}

void ext2_write_bgd(partition_t *part, ext2_bgd_t bgd, uint32_t index) {
    uint64_t bgd_offset = part->ext2_fs->block_size >= 2048 ? part->ext2_fs->block_size : part->ext2_fs->block_size * 2;
    uint64_t group_index = (index - 1) / part->ext2_fs->superblock.inodes_per_group;
    partition_write(part, bgd_offset + (sizeof(ext2_bgd_t) * group_index), sizeof(ext2_bgd_t), &bgd);
}
