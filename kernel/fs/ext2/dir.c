#include <fs/ext2/dir.h>
#include <fs/ext2/ext2.h>
#include <fs/ext2/inode.h>

static int find_dir_relative(devfs_node_t *devfs_node, ext2_inode_t *inode, ext2_dir_t *ret, char *path) {
    void *buffer = kcalloc(inode->size32l);
    ext2_inode_read(devfs_node, inode, 0, inode->size32l, buffer);

    for(uint32_t i = 0; i < inode->size32l; i++) {
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

        if(dir->entry_size != 0) {
            i += dir->entry_size - 1;
        }
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
