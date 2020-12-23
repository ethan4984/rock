#include <fs/ext2/block.h>
#include <bitmap.h>

static uint32_t bgd_find_block(partition_t *part, uint32_t bgd_index) {
    uint8_t *bitmap = kcalloc(part->ext2_fs->block_size);
    ext2_bgd_t bgd = ext2_read_bgd(part, bgd_index);

    partition_read(part, bgd.block_addr_bitmap, part->ext2_fs->block_size, bitmap);
    for(uint32_t i = 0; i < part->ext2_fs->block_size; i++) {
        if(!BM_TEST(bitmap, i)) {
            BM_SET(bitmap, i);
            partition_write(part, bgd.block_addr_bitmap, part->ext2_fs->block_size, bitmap);
            bgd.unallocated_blocks--;
            ext2_write_bgd(part, bgd, bgd_index);
            return i;
        }
    }
    
    kfree(bitmap);
    return -1;
}

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

uint32_t ext2_alloc_block(partition_t *part) {
    for(uint32_t i = 0; i < part->ext2_fs->bgd_cnt; i++) {
        ext2_bgd_t bgd = ext2_read_bgd(part, i);
        if(bgd.unallocated_blocks == 0)
            continue;

        uint32_t free_block = bgd_find_block(part, i);
        if(free_block == (uint32_t)-1) 
            continue;

        return i * part->ext2_fs->superblock.inodes_per_group + free_block;
    }
    return -1;
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
