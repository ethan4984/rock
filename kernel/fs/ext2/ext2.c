#include <fs/device.h>
#include <fs/ext2/ext2.h>
#include <fs/ext2/inode.h>
#include <fs/ext2/block.h>
#include <fs/ext2/dir.h>
#include <output.h>

int ext2_check_fs(devfs_node_t *devfs_node) {
    ext2_fs_t *ext2 = kmalloc(sizeof(ext2_fs_t));

    msd_raw_read(devfs_node, SECTOR_SIZE * 2, sizeof(ext2_superblock_t), &ext2->superblock);

    if(ext2->superblock.signature != 0xef53) {
        kfree(ext2);
        return -1;
    }

    devfs_node->device->fs = kmalloc(sizeof(filesystem_t));
    devfs_node->device->fs->ext2_fs = ext2;

    ext2->block_size = 1024 << ext2->superblock.block_size;
    ext2->frag_size = 1024 << ext2->superblock.frag_size;
    ext2->bgd_cnt = ext2->superblock.block_cnt / ext2->superblock.blocks_per_group;
    ext2->root_inode = ext2_inode_read_entry(devfs_node, 2);

    devfs_node->device->fs->read = ext2_read;
    devfs_node->device->fs->write = ext2_write;
    devfs_node->device->fs->mkdir = ext2_mkdir;
    devfs_node->device->fs->open = ext2_open;
    devfs_node->device->fs->unlink = ext2_unlink;
    devfs_node->device->fs->refresh = ext2_refresh;

    return 0;
}

int ext2_open(vfs_node_t *vfs_node, int flags) {
    devfs_node_t *devfs_node = vfs_node->fs->devfs_node;
    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;

    if(flags & O_CREAT) {
        ext2_inode_t parent_inode;
        uint32_t parent_inode_index;

        ext2_inode_t new_inode;
        uint32_t new_inode_index;

        if(strcmp(vfs_node->parent->relative_path, "/") == 0) {
            parent_inode = ext2->root_inode;
            parent_inode_index = 2;
        } else {
            ext2_dir_t dir;
            if(ext2_find_dir(devfs_node, &ext2->root_inode, &dir, vfs_node->parent->relative_path) == -1)
                return -1;

            parent_inode = ext2_inode_read_entry(devfs_node, dir.inode);
            parent_inode_index = dir.inode;
        }

        new_inode_index = ext2_alloc_inode(devfs_node);
        new_inode = ext2_inode_read_entry(devfs_node, new_inode_index);

        if(inode_set_block(devfs_node, &new_inode, new_inode_index, 0, ext2_alloc_block(devfs_node)) == -1)
            return -1;

        new_inode.sector_cnt = (ext2->block_size / SECTOR_SIZE);
        ext2_inode_write_entry(devfs_node, &new_inode, new_inode_index);
    
        ext2_create_dir(devfs_node, &parent_inode, parent_inode_index, new_inode_index, 0, vfs_node->absolute_path + last_char(vfs_node->absolute_path, '/'));
        parent_inode.hard_link_cnt++;
        ext2_inode_write_entry(devfs_node, &parent_inode, parent_inode_index);

        return 0;
    }

    ext2_dir_t dir;
    if(ext2_find_dir(vfs_node->fs->devfs_node, &vfs_node->fs->ext2_fs->root_inode, &dir, vfs_node->relative_path) == -1)
        return -1;

    ext2_inode_t inode = ext2_inode_read_entry(devfs_node, dir.inode);
    vfs_node->stat.st_size = inode.sector_cnt * SECTOR_SIZE;

    return 0;
}

int ext2_read(vfs_node_t *vfs_node, off_t off, off_t cnt, void *buf) {
    devfs_node_t *devfs_node = vfs_node->fs->devfs_node;
    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;

    if(off > vfs_node->stat.st_size)
        return -1;

    if((off + cnt) > vfs_node->stat.st_size)
        cnt = vfs_node->stat.st_size - off;

    ext2_dir_t dir;
    if(ext2_find_dir(devfs_node, &ext2->root_inode, &dir, vfs_node->relative_path) == -1)
        return -1;

    ext2_inode_t inode = ext2_inode_read_entry(devfs_node, dir.inode);
    ext2_inode_read(devfs_node, &inode, off, cnt, buf);
    return cnt;
}

int ext2_write(vfs_node_t *vfs_node, off_t off, off_t cnt, void *buf) {
    devfs_node_t *devfs_node = vfs_node->fs->devfs_node;
    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;

    if((off + cnt) > vfs_node->stat.st_size)
        vfs_node->stat.st_size += off + cnt - vfs_node->stat.st_size;

    kprintf("[KDEBUG]", "%s", vfs_node->relative_path);

    ext2_dir_t dir;
    if(ext2_find_dir(devfs_node, &ext2->root_inode, &dir, vfs_node->relative_path) == -1)
        return -1;

    ext2_inode_t inode = ext2_inode_read_entry(devfs_node, dir.inode);
    ext2_inode_write(devfs_node, &inode, dir.inode, off, cnt, buf);
    return cnt;
}

static void ext2_refresh_node(devfs_node_t *devfs_node, ext2_inode_t *inode, char *mount_gate) {
    void *buffer = kcalloc(inode->size32l);
    ext2_inode_read(devfs_node, inode, 0, inode->size32l, buffer);

    for(uint32_t i = 0; i < inode->size32l;) {
        ext2_dir_t *dir = (ext2_dir_t*)((uintptr_t)buffer + i);

        char *dir_name = kmalloc(dir->name_length + 1);
        strncpy(dir_name, dir->name, dir->name_length);
        dir_name[dir->name_length] = '\0';
        
        char *absolute_path = str_congregate(mount_gate, dir_name);
        kfree(dir_name);

        if(strcmp(dir_name, ".") == 0 || strcmp(dir_name, "..") == 0) {
            vfs_create_node_deep(absolute_path);
            i += dir->entry_size;
            continue;
        }

        ext2_inode_t dir_inode = ext2_inode_read_entry(devfs_node, dir->inode);

        if(dir_inode.permissions & 0x4000) {
            char *dir_path = str_congregate(absolute_path, "/");
            kfree(absolute_path);
            vfs_create_node_deep(dir_path);
            ext2_refresh_node(devfs_node, &dir_inode, dir_path);
        } else { 
            vfs_create_node_deep(absolute_path);
        }

        uint32_t expected_size = ALIGN_UP(sizeof(ext2_dir_t) + dir->name_length, 4);
        if(dir->entry_size != expected_size) {
            return;
        }

        i += dir->entry_size;
    }
}

int ext2_refresh(vfs_node_t *vfs_node) {
    devfs_node_t *devfs_node = vfs_node->fs->devfs_node;
    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;

    ext2_refresh_node(devfs_node, &ext2->root_inode, devfs_node->device->fs->mount_gate); 

    return 0; 
}

int ext2_unlink(vfs_node_t *vfs_node) {
    devfs_node_t *devfs_node = vfs_node->fs->devfs_node;
    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;

    ext2_delete_dir(devfs_node, &ext2->root_inode, vfs_node->relative_path);

    return 0;
}

int ext2_mkdir(vfs_node_t *vfs_node, uint16_t perms) {
    return 0;
}
