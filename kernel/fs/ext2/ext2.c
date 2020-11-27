#include <fs/ext2/ext2.h>
#include <memutils.h>
#include <output.h>
#include <bitmap.h>

static void inode_read(partition_t *part, ext2_inode_t inode, uint64_t start, uint64_t cnt, void *buffer);
static void inode_write(partition_t *part, ext2_inode_t inode, uint64_t start, uint64_t cnt, void *buffer);
static ext2_inode_t read_inode(partition_t *part, uint32_t index);
static uint32_t alloc_inode(partition_t *part);
static void free_inode(partition_t *part, uint32_t index);

static ext2_bgd_t read_bgd(partition_t *part, uint32_t index);
static void write_bgd(partition_t *part, ext2_bgd_t bgd, uint32_t index);

static void free_block(partition_t *part, uint32_t block); 
static void alloc_block(partition_t *part, uint32_t block); 

static int read_dir(partition_t *part, ext2_inode_t inode, dir_entry_t *ret, char *path);
static int create_dir_entry(partition_t *part, ext2_inode_t parent, uint32_t inode, char *name, uint8_t type); 
static void delete_dir_entry(partition_t *part, ext2_inode_t parent, char *name); 

ext2_fs_t *fs_check_ext2(partition_t part) {
    ext2_fs_t *new_fs = kmalloc(sizeof(ext2_fs_t));

    partition_read(&part, 0x400, sizeof(ext2_superblock_t), &new_fs->superblock);
    if(new_fs->superblock.signature != 0xef53) {
        kfree(new_fs);
        return NULL;
    }
    
    new_fs->block_size = 1024 << new_fs->superblock.block_size;
    new_fs->frag_size = 1024 << new_fs->superblock.frag_size;

    part.ext2_fs = new_fs;
    part.ext2_fs->root_inode = read_inode(&part, 2);
    part.ext2_fs->bgd_cnt = part.ext2_fs->superblock.block_cnt / part.ext2_fs->superblock.blocks_per_group;

    ext2_write(&part, "limine.cfg", 0, 4, "bruh");
    delete_dir_entry(&part, part.ext2_fs->root_inode, "limine.cfg");
    char *buffer = kmalloc(100); 
    ext2_read(&part, "limine.cfg", 0, 100, buffer);
    kprintf("[KDEBUG]", "\n%s", buffer);

    return new_fs;
}

int ext2_read(partition_t *part, char *path, uint64_t start, uint64_t cnt, void *buffer) {
    dir_entry_t dir;
    if(read_dir(part, part->ext2_fs->root_inode, &dir, path) != 1) { 
        return 0;
    }
    inode_read(part, read_inode(part, dir.inode), start, cnt, buffer);
    return cnt;
}

int ext2_write(partition_t *part, char *path, uint64_t start, uint64_t cnt, void *buffer) {
    dir_entry_t dir;
    if(read_dir(part, part->ext2_fs->root_inode, &dir, path) != 1) { 
        return 0;
    }
    inode_write(part, read_inode(part, dir.inode), start, cnt, buffer);
    return cnt;
}

static ext2_bgd_t read_bgd(partition_t *part, uint32_t index) {
    ext2_bgd_t bgd;

    uint64_t bgd_offset = part->ext2_fs->block_size >= 2048 ? part->ext2_fs->block_size : part->ext2_fs->block_size * 2;
    uint64_t group_index = (index - 1) / part->ext2_fs->superblock.inodes_per_group;

    partition_read(part, bgd_offset + (sizeof(ext2_bgd_t) * group_index), sizeof(ext2_bgd_t), &bgd);
    return bgd;
}

static void write_bgd(partition_t *part, ext2_bgd_t bgd, uint32_t index) {
    uint64_t bgd_offset = part->ext2_fs->block_size >= 2048 ? part->ext2_fs->block_size : part->ext2_fs->block_size * 2;
    uint64_t group_index = (index - 1) / part->ext2_fs->superblock.inodes_per_group;
    partition_write(part, bgd_offset + (sizeof(ext2_bgd_t) * group_index), sizeof(ext2_bgd_t), &bgd);
}

static ext2_inode_t read_inode(partition_t *part, uint32_t index) {
    ext2_bgd_t bgd = read_bgd(part, index);

    uint64_t inode_table_index = (index - 1) % part->ext2_fs->superblock.inodes_per_group;
    ext2_inode_t inode;

    partition_read(part, (bgd.inode_table_block * part->ext2_fs->block_size) + (part->ext2_fs->superblock.inode_size * inode_table_index), sizeof(ext2_inode_t), &inode);
    return inode;
}

static void write_inode(partition_t *part, ext2_inode_t inode, uint32_t index) {
    ext2_bgd_t bgd = read_bgd(part, index);
    uint64_t inode_table_index = (index - 1) % part->ext2_fs->superblock.inodes_per_group;
    partition_write(part, (bgd.inode_table_block * part->ext2_fs->block_size) + (part->ext2_fs->superblock.inode_size * inode_table_index), sizeof(ext2_inode_t), &inode);
}

static void inode_read(partition_t *part, ext2_inode_t inode, uint64_t start, uint64_t cnt, void *buffer) {
    uint64_t block_size = part->ext2_fs->block_size, headway = 0;

    while(headway < cnt) {
        uint64_t block = (start + headway) / block_size;

        uint64_t size = cnt - headway;
        uint64_t offset = (start + headway) % block_size;

        if(size > block_size - offset)
            size = block_size - offset;

        uint32_t block_index;

        if(block < 12) { // direct
            block_index = inode.blocks[block];
        } else { // indirect
            block -= 12;
            if(block * sizeof(uint32_t) >= block_size) { // double indirect
                block -= block_size / sizeof(uint32_t);
                uint32_t index = block / (block_size / sizeof(uint32_t));
                if(index * sizeof(uint32_t) >= block_size) { // triple indirect
                    
                }
                uint32_t offset = block % (block_size / sizeof(uint32_t));
                uint32_t indirect_block;

                partition_read(part, (inode.blocks[13] * block_size) + (index * sizeof(uint32_t)), sizeof(uint32_t), &indirect_block);
                partition_read(part, indirect_block * block_size + offset, sizeof(uint32_t), &block_index);
            } else {
                partition_read(part, (inode.blocks[12] * block_size) + (block * sizeof(uint32_t)), sizeof(uint32_t), &block_index);
            }
        }
        partition_read(part, block_index * block_size + offset, size, (void*)((uint64_t)buffer + headway));

        headway += size;
    }
}

static void inode_write(partition_t *part, ext2_inode_t inode, uint64_t start, uint64_t cnt, void *buffer) {
    uint64_t block_size = part->ext2_fs->block_size, headway = 0;

    while(headway < cnt) {
        uint64_t block = (start + headway) / block_size;

        uint64_t size = cnt - headway;
        uint64_t offset = (start + headway) % block_size;

        if(size > block_size - offset)
            size = block_size - offset;

        uint32_t block_index;

        if(block < 12) { // direct
            block_index = inode.blocks[block];
        } else { // indirect
            block -= 12;
            if(block * sizeof(uint32_t) >= block_size) { // double indirect
                block -= block_size / sizeof(uint32_t);
                uint32_t index = block / (block_size / sizeof(uint32_t));
                if(index * sizeof(uint32_t) >= block_size) { // triple indirect
                    
                }
                uint32_t offset = block % (block_size / sizeof(uint32_t));
                uint32_t indirect_block;

                partition_read(part, (inode.blocks[13] * block_size) + (index * sizeof(uint32_t)), sizeof(uint32_t), &indirect_block);
                partition_read(part, indirect_block * block_size + offset, sizeof(uint32_t), &block_index);
            } else {
                partition_read(part, (inode.blocks[12] * block_size) + (block * sizeof(uint32_t)), sizeof(uint32_t), &block_index);
            }
        }

        alloc_block(part, block_index);
        partition_write(part, block_index * block_size + offset, size, (void*)((uint64_t)buffer + headway));

        headway += size;
    }
}

static void free_block(partition_t *part, uint32_t block) {
    uint8_t *bitmap = kcalloc(part->ext2_fs->block_size);
    uint32_t bgd_index = block / part->ext2_fs->superblock.blocks_per_group; 
    uint32_t bitmap_index = block - (block / part->ext2_fs->superblock.blocks_per_group);

    ext2_bgd_t bgd = read_bgd(part, bgd_index);

    partition_read(part, bgd.block_addr_bitmap, part->ext2_fs->block_size, bitmap);
    if(!BM_TEST(bitmap, bitmap_index)) {
        kfree(bitmap);
        return;
    }

    BM_CLEAR(bitmap, bitmap_index);
    partition_write(part, bgd.block_addr_bitmap, part->ext2_fs->block_size, bitmap);
    bgd.unallocated_blocks++;
    write_bgd(part, bgd, bgd_index);
    kfree(bitmap);
}

static void alloc_block(partition_t *part, uint32_t block) {
    uint8_t *bitmap = kcalloc(part->ext2_fs->block_size);
    uint32_t bgd_index = block / part->ext2_fs->superblock.blocks_per_group; 
    uint32_t bitmap_index = block - (block / part->ext2_fs->superblock.blocks_per_group);

    ext2_bgd_t bgd = read_bgd(part, bgd_index);

    partition_read(part, bgd.block_addr_bitmap, part->ext2_fs->block_size, bitmap);
    if(BM_TEST(bitmap, bitmap_index)) {
        kfree(bitmap);
        return;
    }

    BM_SET(bitmap, bitmap_index);
    partition_write(part, bgd.block_addr_bitmap, part->ext2_fs->block_size, bitmap);
    bgd.unallocated_blocks++;
    write_bgd(part, bgd, bgd_index);
    kfree(bitmap);
} 

static uint32_t alloc_inode(partition_t *part) {
    uint8_t *bitmap = kcalloc(part->ext2_fs->block_size);

    for(uint32_t i = 0; i < part->ext2_fs->bgd_cnt; i++) {
        ext2_bgd_t bgd = read_bgd(part, i);
        if(!bgd.unallocated_inodes)
            continue; 

        partition_read(part, bgd.block_addr_inode, part->ext2_fs->block_size, bitmap);
        for(uint32_t j = 0; j < part->ext2_fs->block_size; i++) {
            if(!BM_TEST(bitmap, j)) {
                BM_SET(bitmap, j);
                partition_write(part, bgd.block_addr_inode, part->ext2_fs->block_size, bitmap);

                bgd.unallocated_inodes--;
                write_bgd(part, bgd, i);

                return i * part->ext2_fs->superblock.inodes_per_group + j;
            }
        }
    }
    return -1;
}

static void free_inode(partition_t *part, uint32_t index) {
    uint8_t *bitmap = kcalloc(part->ext2_fs->block_size);
    uint32_t bgd_index = index / part->ext2_fs->superblock.inodes_per_group;
    uint32_t bitmap_index = index - (index / part->ext2_fs->superblock.inodes_per_group);

    ext2_bgd_t bgd = read_bgd(part, bgd_index);

    partition_read(part, bgd.block_addr_inode, part->ext2_fs->block_size, bitmap);
    if(!BM_TEST(bitmap, bitmap_index)) { 
        kfree(bitmap);
        return;
    }

    BM_CLEAR(bitmap, bitmap_index);
    partition_write(part, bgd.block_addr_inode, part->ext2_fs->block_size, bitmap);
    bgd.unallocated_inodes++;
    write_bgd(part, bgd, bgd_index);
    kfree(bitmap);
}

static void delete_dir_entry(partition_t *part, ext2_inode_t parent, char *name) { 
    void *buffer = kmalloc(parent.size32l);
    inode_read(part, parent, 0, parent.size32l, buffer); 

    for(uint32_t i = 0; i < parent.size32l; i++) {
        dir_entry_t *dir = (dir_entry_t*)((uint64_t)buffer + i);

        if((strncmp(dir->name, name, strlen(name)) == 0)) {
            memset8((uint8_t*)dir, 0, dir->entry_size);
            inode_write(part, parent, 0, parent.size32l, buffer);
            return; 
        }

        if(dir->entry_size)
            i += dir->entry_size - 1;
    }
}

static int create_dir_entry(partition_t *part, ext2_inode_t parent, uint32_t inode, char *name, uint8_t type) {
    void *buffer = kmalloc(parent.size32l);
    inode_read(part, parent, 0, parent.size32l, buffer);

    int found = 0;

    for(uint32_t i = 0; i < parent.size32l; i++) {
        dir_entry_t *dir = (dir_entry_t*)((uint64_t)buffer + i);

        if(found) {
            dir->name_length = strlen(name);
            dir->type = type;
            dir->inode = inode; 
            inode_write(part, parent,0, parent.size32l, buffer);
            return 1; 
        } 

        if(strncmp(dir->name, name, strlen(name)) == 0) {
            kprintf("[FS]", "%s already exists", name);
            kfree(buffer);
            return 0;
        }

        uint32_t expected_size = sizeof(dir_entry_t) + dir->name_length;
        if(expected_size != dir->entry_size) {
            i += expected_size - 1;
            found = 1;
            continue;
        }
        
        i = dir->entry_size - 1;
    }
    kfree(buffer);
    return 1;
}

static int read_dir(partition_t *part, ext2_inode_t inode, dir_entry_t *ret, char *path) { // TODO make thread save
    if(*path == '/')
        ++path;

    void *buffer = kcalloc(inode.size32l);
    inode_read(part, inode, 0, inode.size32l, buffer);

    int cnt = 0, parse_cnt = character_cnt(path, '/');

    char *sub_path = strtok(path, "/");
    while(sub_path != NULL) {
        for(uint32_t i = 0; i < inode.size32l; i++) {
            dir_entry_t *dir = (dir_entry_t*)((uint64_t)buffer + i);

            if((strncmp(dir->name, sub_path, strlen(sub_path)) == 0) && cnt == parse_cnt) {
                *ret = *dir;
                goto end;
            }

            if(strncmp(dir->name, sub_path, strlen(sub_path)) == 0) {
                inode = read_inode(part, dir->inode);
                if(!(inode.permissions & 0x4000)) {
                    kprintf("[KDEBUG]", "%s is not a directory", sub_path);
                    kfree(sub_path),
                    kfree(buffer);
                    return 0;
                }
                inode_read(part, inode, 0, inode.size32l, buffer);
                break;
            }

            if(dir->entry_size != 0) {
                i += dir->entry_size - 1;
            }
        }
        kfree(sub_path);
        sub_path = strtok(NULL, "/");
        cnt++;
    }

    kfree(buffer);
    kprintf("[KDEBUG]", "%s not found", path);
    return 0;
end:
    kfree(sub_path);
    kfree(buffer);
    return 1;
}
