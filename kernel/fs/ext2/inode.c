#include <fs/ext2/inode.h>
#include <fs/ext2/block.h>
#include <bitmap.h>

uint32_t ext2_alloc_inode(partition_t *part) {
    uint8_t *bitmap = kcalloc(part->ext2_fs->block_size);

    for(uint32_t i = 3; i < part->ext2_fs->bgd_cnt; i++) {
        ext2_bgd_t bgd = ext2_read_bgd(part, i);
        if(!bgd.unallocated_inodes)
            continue; 

        partition_read(part, bgd.block_addr_inode, part->ext2_fs->block_size, bitmap);
        for(uint32_t j = 0; j < part->ext2_fs->block_size; i++) {
            if(!BM_TEST(bitmap, j)) {
                BM_SET(bitmap, j);
                partition_write(part, bgd.block_addr_inode, part->ext2_fs->block_size, bitmap);

                bgd.unallocated_inodes--;
                ext2_write_bgd(part, bgd, i);

                return i * part->ext2_fs->superblock.inodes_per_group + j;
            }
        }
    }
    return -1;
}


void ext2_free_inode(partition_t *part, uint32_t index) {
    uint8_t *bitmap = kcalloc(part->ext2_fs->block_size);
    uint32_t bgd_index = index / part->ext2_fs->superblock.inodes_per_group;
    uint32_t bitmap_index = index - (index / part->ext2_fs->superblock.inodes_per_group);

    ext2_bgd_t bgd = ext2_read_bgd(part, bgd_index);

    partition_read(part, bgd.block_addr_inode, part->ext2_fs->block_size, bitmap);
    if(!BM_TEST(bitmap, bitmap_index)) { 
        kfree(bitmap);
        return;
    }

    BM_CLEAR(bitmap, bitmap_index);
    partition_write(part, bgd.block_addr_inode, part->ext2_fs->block_size, bitmap);
    bgd.unallocated_inodes++;
    ext2_write_bgd(part, bgd, bgd_index);
    kfree(bitmap);
}

static void inode_resize(partition_t *part, ext2_inode_t *inode, uint32_t previous_size) {
    
} 

void ext2_inode_read(partition_t *part, ext2_inode_t *inode, uint64_t start, uint64_t cnt, void *buffer) {
    uint64_t block_size = part->ext2_fs->block_size, headway = 0;

    while(headway < cnt) {
        uint64_t block = (start + headway) / block_size;

        uint64_t size = cnt - headway;
        uint64_t offset = (start + headway) % block_size;

        if(size > block_size - offset)
            size = block_size - offset;

        uint32_t block_index;

        if(block < 12) { // direct
            block_index = inode->blocks[block];
        } else { // indirect
            block -= 12;
            if(block * sizeof(uint32_t) >= block_size) { // double indirect
                block -= block_size / sizeof(uint32_t);
                uint32_t index = block / (block_size / sizeof(uint32_t));
                if(index * sizeof(uint32_t) >= block_size) { // triple indirect
                    
                }
                uint32_t offset = block % (block_size / sizeof(uint32_t));
                uint32_t indirect_block;

                partition_read(part, inode->blocks[13] * block_size + index * sizeof(uint32_t), sizeof(uint32_t), &indirect_block);
                partition_read(part, indirect_block * block_size + offset * sizeof(uint32_t), sizeof(uint32_t), &block_index);
            } else {
                partition_read(part, (inode->blocks[12] * block_size) + (block * sizeof(uint32_t)), sizeof(uint32_t), &block_index);
            }
        }
        partition_read(part, block_index * block_size + offset, size, (void*)((uint64_t)buffer + headway));

        headway += size;
    }
}

void ext2_inode_write(partition_t *part, ext2_inode_t *inode, uint64_t start, uint64_t cnt, void *buffer) {
    if(inode->size32h > start + cnt) {
        uint32_t previous_size = inode->size32h;
        inode->size32h = start + cnt;
        inode_resize(part, inode, previous_size);     
    }

    uint64_t block_size = part->ext2_fs->block_size, headway = 0;

    while(headway < cnt) {
        uint64_t block = (start + headway) / block_size;

        uint64_t size = cnt - headway;
        uint64_t offset = (start + headway) % block_size;

        if(size > block_size - offset)
            size = block_size - offset;

        uint32_t block_index;

        if(block < 12) { // direct
            block_index = inode->blocks[block];
        } else { // indirect
            block -= 12;
            if(block * sizeof(uint32_t) >= block_size) { // double indirect
                block -= block_size / sizeof(uint32_t);
                uint32_t index = block / (block_size / sizeof(uint32_t));
                if(index * sizeof(uint32_t) >= block_size) { // triple indirect
                    
                }
                uint32_t offset = block % (block_size / sizeof(uint32_t));
                uint32_t indirect_block;

                partition_read(part, (inode->blocks[13] * block_size) + (index * sizeof(uint32_t)), sizeof(uint32_t), &indirect_block);
                partition_read(part, indirect_block * block_size + offset, sizeof(uint32_t), &block_index);
            } else {
                partition_read(part, (inode->blocks[12] * block_size) + (block * sizeof(uint32_t)), sizeof(uint32_t), &block_index);
            }
        }

        partition_write(part, block_index * block_size + offset, size, (void*)((uint64_t)buffer + headway));

        headway += size;
    }
}

ext2_inode_t ext2_inode_read_entry(partition_t *part, uint32_t index) {
    ext2_bgd_t bgd = ext2_read_bgd(part, index);

    uint64_t inode_table_index = (index - 1) % part->ext2_fs->superblock.inodes_per_group;
    ext2_inode_t inode;

    partition_read(part, (bgd.inode_table_block * part->ext2_fs->block_size) + (part->ext2_fs->superblock.inode_size * inode_table_index), sizeof(ext2_inode_t), &inode);
    return inode;
}

void ext2_inode_write_entry(partition_t *part, uint32_t index, ext2_inode_t *inode) {
    ext2_bgd_t bgd = ext2_read_bgd(part, index);

    uint64_t inode_table_index = (index - 1) % part->ext2_fs->superblock.inodes_per_group;
    partition_read(part, (bgd.inode_table_block * part->ext2_fs->block_size) + (part->ext2_fs->superblock.inode_size * inode_table_index), sizeof(ext2_inode_t), inode);
}

void ext2_alloc_inode_block(partition_t *part, ext2_inode_t *inode, uint32_t inode_index, uint32_t block) {
    
}

void ext2_free_inode_block(partition_t *part, ext2_inode_t *inode, uint32_t inode_index, uint32_t block) {

}
