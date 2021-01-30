#include <fs/ext2/inode.h>
#include <fs/ext2/block.h>

uint32_t ext2_alloc_inode(devfs_node_t *devfs_node) {
    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;

    uint8_t *bitmap = kcalloc(ext2->block_size);
    for(uint32_t i = 3; i < ext2->bgd_cnt; i++) {
        ext2_bgd_t bgd = ext2_read_bgd(devfs_node, i);
        if(!bgd.unallocated_inodes)
            continue;

        msd_raw_read(devfs_node, bgd.block_addr_inode, ext2->block_size, bitmap);
        for(uint32_t j = 0; j < ext2->block_size; j++) {
            if(!BM_TEST(bitmap, j)) {
                BM_SET(bitmap, j);
                msd_raw_write(devfs_node, bgd.block_addr_inode, ext2->block_size, bitmap);

                bgd.unallocated_inodes--;
                ext2_write_bgd(devfs_node, &bgd, i);

                kfree(bitmap);
                return i * ext2->superblock.inodes_per_group + j;
            }
        }
    }

    kfree(ext2);
    return -1;
}

void ext2_free_inode(devfs_node_t *devfs_node, uint32_t index) {
    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;

    uint8_t *bitmap = kcalloc(ext2->block_size);
    uint32_t bgd_index = index / ext2->superblock.inodes_per_group;
    uint32_t bitmap_index = index - (index / ext2->superblock.inodes_per_group);

    ext2_bgd_t bgd = ext2_read_bgd(devfs_node, bgd_index);

    msd_raw_read(devfs_node, bgd.block_addr_inode, ext2->block_size, bitmap);
    if(!BM_TEST(bitmap, bitmap_index)) {
        kfree(bitmap);
        return;
    }

    BM_CLEAR(bitmap, bitmap_index);
    msd_raw_write(devfs_node, bgd.block_addr_inode, ext2->block_size, bitmap);

    bgd.unallocated_inodes++;
    ext2_write_bgd(devfs_node, &bgd, bgd_index);
    kfree(bitmap);
}

static uint32_t inode_get_block(devfs_node_t *devfs_node, ext2_inode_t *inode, uint32_t iblock) {
    uint32_t block_size = devfs_node->device->fs->ext2_fs->block_size, block_index;
    if(iblock < 12) { // direct
        block_index = inode->blocks[iblock];
    } else { 
        iblock -= 12;
        if(iblock * sizeof(uint32_t) >= block_size) { // double indirect
            iblock -= block_size / sizeof(uint32_t);
            uint32_t index = iblock / (block_size / sizeof(uint32_t));
            if(index * sizeof(uint32_t) >= block_size) { // triple indirect
                //TODO implement this
            }
            uint32_t indirect_offset = iblock % (block_size / sizeof(uint32_t));
            uint32_t indirect_block;

            msd_raw_read(devfs_node, inode->blocks[13] * block_size + index * sizeof(uint32_t), sizeof(uint32_t), &indirect_block);
            msd_raw_read(devfs_node, (indirect_block * block_size) + (indirect_offset * sizeof(uint32_t)), sizeof(uint32_t), &block_index);
        } else {
            msd_raw_read(devfs_node, (inode->blocks[12] * block_size) + (iblock * sizeof(uint32_t)), sizeof(uint32_t), &block_index);
        }
    }
    return block_index;
}

static int inode_set_block(devfs_node_t *devfs_node, ext2_inode_t *inode, uint32_t inode_index, uint32_t iblock, uint32_t disk_block) {
    uint32_t block_size = devfs_node->device->fs->ext2_fs->block_size;
    if(iblock < 12) { // direct
        inode->blocks[iblock] = disk_block;
    } else { 
        iblock -= 12;
        if(iblock * sizeof(uint32_t) >= block_size) { // double indirect
            iblock -= block_size / sizeof(uint32_t);
            uint32_t index = iblock / (block_size / sizeof(uint32_t));
            if(index * sizeof(uint32_t) >= block_size) { // triple indirect
                //TODO implement this
            }
            uint32_t indirect_offset = iblock % (block_size / sizeof(uint32_t));
            uint32_t indirect_block;

            if(!inode->blocks[13]) {
                uint32_t block = ext2_alloc_block(devfs_node);
                if(block == (uint32_t)-1)
                    return -1;

                inode->blocks[13] = block;
                ext2_inode_write_entry(devfs_node, inode, inode_index);
            }

            msd_raw_read(devfs_node, inode->blocks[13] * block_size + index * sizeof(uint32_t), sizeof(uint32_t), &indirect_block);
            if(!indirect_block) {
                uint32_t block = ext2_alloc_block(devfs_node);
                if(block == (uint32_t)-1)
                    return -1;
                indirect_block = block;
                msd_raw_write(devfs_node, inode->blocks[13] * block_size + index * sizeof(uint32_t), sizeof(uint32_t), &indirect_block);
            }

            msd_raw_write(devfs_node, (indirect_block * block_size) + (indirect_offset * sizeof(uint32_t)), sizeof(uint32_t), &disk_block);
        } else {
            if(!inode->blocks[12]) {
                uint32_t block = ext2_alloc_block(devfs_node);
                if(block == (uint32_t)-1)
                    return -1;

                inode->blocks[12] = block;
                ext2_inode_write_entry(devfs_node, inode, inode_index);
            }

            msd_raw_write(devfs_node, (inode->blocks[12] * block_size) + (iblock * sizeof(uint32_t)), sizeof(uint32_t), &disk_block);
        }
    }
    return 0;
}

static int inode_resize(devfs_node_t *devfs_node, ext2_inode_t *inode, uint32_t inode_index, off_t start, off_t cnt) {
    if((start + cnt) < (inode->sector_cnt * SECTOR_SIZE))
        return 0;

    uint32_t block_size = devfs_node->device->fs->ext2_fs->block_size;
    uint32_t iblock_start = ROUNDUP(inode->sector_cnt * SECTOR_SIZE, block_size);
    uint32_t iblock_end = ROUNDUP(start + cnt, block_size);

    for(size_t i = iblock_start; i < iblock_end; i++) {
        uint32_t disk_block = ext2_alloc_block(devfs_node);
        if(disk_block == (uint32_t)-1)
            return -1;

        if(inode_set_block(devfs_node, inode, inode_index, i, disk_block) == -1)
            return -1;
    }

    return 0;
}

void ext2_inode_read(devfs_node_t *devfs_node, ext2_inode_t *inode, off_t off, off_t cnt, void *buf) {
    off_t headway = 0, block_size = devfs_node->device->fs->ext2_fs->block_size;

    while(headway < cnt) {
        uint32_t iblock = (off + headway) / block_size;

        size_t size = cnt - headway;
        size_t offset = (off + headway) % block_size;

        if(size > block_size - offset)
            size = block_size - offset;

        uint32_t disk_block = inode_get_block(devfs_node, inode, iblock);

        msd_raw_read(devfs_node, disk_block * block_size + offset, size, (void*)((size_t)buf + headway));

        headway += size;
    }	
}

void ext2_inode_write(devfs_node_t *devfs_node, ext2_inode_t *inode, uint32_t inode_index, off_t off, off_t cnt, void *buf) {
    off_t headway = 0, block_size = devfs_node->device->fs->ext2_fs->block_size;

    inode_resize(devfs_node, inode, inode_index, off, cnt);

    while(headway < cnt) {
        uint32_t iblock = (off + headway) / block_size;

        size_t size = cnt - headway;
        size_t offset = (off + headway) % block_size;

        if(size > block_size - offset)
            size = block_size - offset;

        uint32_t disk_block = inode_get_block(devfs_node, inode, iblock);

        msd_raw_write(devfs_node, disk_block * block_size + offset, size, (void*)((size_t)buf + headway));

        headway += size;
    }
}

void ext2_inode_delete(devfs_node_t *devfs_node, ext2_inode_t *inode, uint32_t inode_index) {
    uint32_t block_size = devfs_node->device->fs->ext2_fs->block_size;

    for(size_t i = 0; i < ROUNDUP(inode->sector_cnt * SECTOR_SIZE, block_size); i++)
        ext2_free_block(devfs_node, inode_get_block(devfs_node, inode, i));

    ext2_free_inode(devfs_node, inode_index);
}

ext2_inode_t ext2_inode_read_entry(devfs_node_t *devfs_node, uint32_t index) {
    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;
    ext2_bgd_t bgd = ext2_read_bgd(devfs_node, index);

    size_t inode_table_index = (index - 1) % ext2->superblock.inodes_per_group;
    ext2_inode_t inode;

    msd_raw_read(devfs_node, (bgd.inode_table_block * ext2->block_size) + (ext2->superblock.inode_size * inode_table_index), sizeof(ext2_inode_t), &inode); 
    return inode;
}

void ext2_inode_write_entry(devfs_node_t *devfs_node, ext2_inode_t *inode, uint32_t index) {
    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;
    ext2_bgd_t bgd = ext2_read_bgd(devfs_node, index);

    size_t inode_table_index = (index - 1) % ext2->superblock.inodes_per_group;
    msd_raw_write(devfs_node, (bgd.inode_table_block * ext2->block_size) + (ext2->superblock.inode_size * inode_table_index), sizeof(ext2_inode_t), &inode); 
}
