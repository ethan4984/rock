#include <fs/ext2/block.h>
#include <fs/ext2/dir.h>
#include <fs/ext2/inode.h>
#include <fs/ext2/ext2.h>
#include <memutils.h>
#include <output.h>
#include <bitmap.h>

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
    part.ext2_fs->root_inode = ext2_inode_read_entry(&part, 2);
    part.ext2_fs->bgd_cnt = part.ext2_fs->superblock.block_cnt / part.ext2_fs->superblock.blocks_per_group;

    return new_fs;
}

int ext2_read(partition_t *part, char *path, uint64_t start, uint64_t cnt, void *buffer) {
    ext2_dir_entry_t dir;
    if(ext2_read_dir_entry(part, &part->ext2_fs->root_inode, &dir, path) == -1) { 
        kprintf("[KDEBUG]", "\"%s\" does not exist", path);
        return 0;
    }

    ext2_inode_t inode = ext2_inode_read_entry(part, dir.inode);
    ext2_inode_read(part, &inode, start, cnt, buffer);
    return cnt;
}

int ext2_write(partition_t *part, char *path, uint64_t start, uint64_t cnt, void *buffer) {
    ext2_dir_entry_t dir;
    if(ext2_read_dir_entry(part, &part->ext2_fs->root_inode, &dir, path) == -1) { 
        kprintf("[KDEBUG]", "\"%s\" does not exist", path);
        return 0;
    }

    ext2_inode_t inode = ext2_inode_read_entry(part, dir.inode);
    ext2_inode_write(part, &inode, start, cnt, buffer);
    return cnt;
}

int ext2_mkdir(partition_t *part, char *parent, char *name, uint16_t permissions) {
    ext2_inode_t parent_inode;
    int parent_inode_index;

    if(strcmp(parent, "/") != 0) {
        ext2_dir_entry_t dir;
        ext2_read_dir_entry(part, &part->ext2_fs->root_inode, &dir, parent);
        parent_inode = ext2_inode_read_entry(part, dir.inode);
        parent_inode_index = dir.inode;
    } else {
        parent_inode = part->ext2_fs->root_inode;
        parent_inode_index = 2; 
    }

    uint32_t inode_index = ext2_alloc_inode(part);

    ext2_inode_t new_inode = {  .permissions = 0x4000 | (0xfff & permissions),
                                .hard_link_cnt = 2,
                                .size32l = part->ext2_fs->block_size
                             };

    parent_inode.hard_link_cnt++;
    ext2_inode_write_entry(part, parent_inode_index, &parent_inode);

    ext2_inode_write_entry(part, inode_index, &new_inode);
    ext2_create_dir_entry(part, &parent_inode, inode_index, name, 0);
    return 0;
}

int ext2_touch(partition_t *part, char *parent, char *name, uint16_t permissions) {
    ext2_inode_t parent_inode;
    int parent_inode_index;

    if(strcmp(parent, "/") != 0) {
        ext2_dir_entry_t dir;
        ext2_read_dir_entry(part, &part->ext2_fs->root_inode, &dir, parent);
        parent_inode = ext2_inode_read_entry(part, dir.inode);
        parent_inode_index = dir.inode;
    } else {
        parent_inode = part->ext2_fs->root_inode;
        parent_inode_index = 2; 
    }

    uint32_t inode_index = ext2_alloc_inode(part);

    ext2_inode_t new_inode = {  .permissions = 0x8000 | (0xfff & permissions),
                                .hard_link_cnt = 2,
                                .size32l = part->ext2_fs->block_size
                             };

    parent_inode.hard_link_cnt++;
    ext2_inode_write_entry(part, parent_inode_index, &parent_inode);

    ext2_inode_write_entry(part, inode_index, &new_inode);
    ext2_create_dir_entry(part, &parent_inode, inode_index, name, 0);
    return 0;
}
