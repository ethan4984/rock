#ifndef _EXT2_DIR_H_
#define _EXT2_DIR_H_

#include <fs/ext2/types.h>
#include <fs/ext2/inode.h>
#include <fs/ext2/block.h>
#include <fs/vfs.h>

int ext2_read_dir_entry(partition_t *part, ext2_inode_t *parent, ext2_dir_entry_t *ret, char *path);

int ext2_create_dir_entry(partition_t *part, ext2_inode_t *parent, uint32_t inode, char *name, uint8_t type); 

void ext2_delete_dir_entry(partition_t *part, ext2_inode_t *parent, char *name); 

#endif
