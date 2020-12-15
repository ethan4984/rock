#ifndef _EXT2_INODE_H_
#define _EXT2_INODE_H_

#include <fs/ext2/types.h>
#include <fs/vfs.h>

void ext2_inode_read(partition_t *part, ext2_inode_t inode, uint64_t start, uint64_t cnt, void *buffer);

void ext2_inode_write(partition_t *part, ext2_inode_t inode, uint64_t start, uint64_t cnt, void *buffer);

ext2_inode_t ext2_inode_read_entry(partition_t *part, uint32_t index);

void ext2_inode_write_entry(partition_t *part, uint32_t index, ext2_inode_t *inode);

uint32_t ext2_alloc_inode(partition_t *part);

void ext2_free_inode(partition_t *part, uint32_t index);

#endif
