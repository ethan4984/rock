#ifndef EXT2_BLOCKS_H_
#define EXT2_BLOCKS_H_

#include <fs/device.h>
#include <fs/ext2/types.h>

struct ext2_bgd ext2_read_bgd(struct devfs_node *devfs_node, uint32_t index);
void ext2_write_bgd(struct devfs_node *devfs_node, struct ext2_bgd *bgd, uint32_t index);

void ext2_free_block(struct devfs_node *devfs_node, uint32_t block);
uint32_t ext2_alloc_block(struct devfs_node *devfs_node);

#endif
