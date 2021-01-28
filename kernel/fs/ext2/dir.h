#ifndef EXT2_DIR_H_
#define EXT2_DIR_H_

#include <fs/ext2/types.h>
#include <fs/device.h>

int ext2_find_dir(devfs_node_t *devfs_node, ext2_inode_t *parent, ext2_dir_t *ret, char *path);

#endif
