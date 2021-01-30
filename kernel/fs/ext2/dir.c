#include <fs/ext2/dir.h>
#include <fs/ext2/ext2.h>
#include <fs/ext2/inode.h>

static int find_dir_relative(devfs_node_t *devfs_node, ext2_inode_t *inode, ext2_dir_t *ret, char *path) {
    void *buffer = kmalloc(inode->size32l);
    ext2_inode_read(devfs_node, inode, 0, inode->size32l, buffer);

    for(uint32_t i = 0; i < inode->size32l;) {
        ext2_dir_t *dir = (ext2_dir_t*)((uint64_t)buffer + i);

        if(strncmp(dir->name, path, strlen(path)) == 0) {
            if(dir->inode == 0) {
                kfree(buffer);
                return -1;
            }
            *ret = *dir;
            kfree(buffer);
            return 0;
        }

        uint32_t expected_size = ALIGN_UP(sizeof(ext2_dir_t) + dir->name_length, 4);
        if(dir->entry_size != expected_size)
            break;

        i += dir->entry_size;
    }
    return -1;
}

int ext2_find_dir(devfs_node_t *devfs_node, ext2_inode_t *inode, ext2_dir_t *ret, char *path) {
    char *cpath = kmalloc(strlen(path));
    strcpy(cpath, path);

    ext2_inode_t used_inode = *inode;

    char *sub_path, *save = cpath;
    while((sub_path = strtok_r(save, "/", &save))) {
        if(find_dir_relative(devfs_node, &used_inode, ret, sub_path) == -1) {
            kfree(cpath);
            return -1;
        }
        used_inode = ext2_inode_read_entry(devfs_node, ret->inode);
    }

    kfree(cpath);
    return 0;
}

int ext2_create_dir(devfs_node_t *devfs_node, ext2_inode_t *parent, uint32_t parent_index, uint32_t inode, uint8_t type, char *name) {
    void *buffer = kmalloc(parent->size32l);
    ext2_inode_read(devfs_node, parent, 0, parent->size32l, buffer);

    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;

    int found = 0;

    for(uint32_t i = 0; i < parent->size32l;) {
        ext2_dir_t *dir = (ext2_dir_t*)((uint64_t)buffer + i);

        if(found) {
            dir->inode = inode;
            dir->type = type;
            dir->name_length = strlen(name);
            dir->entry_size = ext2->block_size - i;
            memcpy8((uint8_t*)dir->name, (uint8_t*)name, dir->name_length);

            i += dir->entry_size;
            dir = (ext2_dir_t*)((uint64_t)buffer + i);
            memset8((uint8_t*)dir, 0, sizeof(ext2_dir_t));

            ext2_inode_write(devfs_node, parent, parent_index, 0, parent->size32l, buffer);

            return 0;
        }

        uint32_t expected_size = ALIGN_UP(sizeof(ext2_dir_t) + dir->name_length, 4);
        if(dir->entry_size != expected_size) {
            dir->entry_size = expected_size;
            i += expected_size;
            found = 1;
            continue;
        }
        i += dir->entry_size;
    }

    return -1;
}

static int ext2_delete_dir_entry(devfs_node_t *devfs_node, ext2_inode_t *parent, uint32_t parent_index, char *name) {
    void *buffer = kmalloc(parent->size32l);
    ext2_inode_read(devfs_node, parent, 0, parent->size32l, buffer);

    ext2_fs_t *ext2 = devfs_node->device->fs->ext2_fs;

    for(uint32_t i = 0; i < parent->size32l;) {
        ext2_dir_t *dir = (ext2_dir_t*)((uint64_t)buffer + i);

        if(strncmp(dir->name, name, strlen(name)) == 0) {
            memset8((uint8_t*)dir->name, 0, dir->name_length);
            ext2_inode_t inode = ext2_inode_read_entry(devfs_node, dir->inode);
            ext2_inode_delete(devfs_node, &inode, dir->inode);
            dir->inode = 0;

            ext2_inode_write(devfs_node, parent, parent_index, 0, parent->size32l, buffer);

            return 0;
        }

        uint32_t expected_size = ALIGN_UP(sizeof(ext2_dir_t) + dir->name_length, 4);
        if(dir->entry_size != expected_size)
            break;

        i += dir->entry_size;
    }
    return -1;
}

int ext2_delete_dir(devfs_node_t *devfs_node, ext2_inode_t *parent, char *path) {
    char *cpath = kmalloc(strlen(path));
    strcpy(cpath, path);

    ext2_inode_t inode = *parent;
    uint32_t inode_index;
    ext2_dir_t dir;

    char *sub_path, *save = cpath;
    while((sub_path = strtok_r(save, "/", &save))) {
        if(find_dir_relative(devfs_node, &inode, &dir, sub_path) == -1) {
            kfree(cpath);
            return -1;
        }
        inode = ext2_inode_read_entry(devfs_node, dir.inode);
        inode_index = dir.inode;
    }

    kfree(cpath);
    return ext2_delete_dir_entry(devfs_node, &inode, inode_index, path + last_char(path, '/'));
}
