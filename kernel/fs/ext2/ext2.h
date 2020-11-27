#ifndef EXT2_SUPERBLOCK_H_
#define EXT2_SUPERBLOCK_H_

#include <fs/vfs.h>

ext2_fs_t *fs_check_ext2(partition_t part);

int ext2_read(partition_t *part, char *path, uint64_t start, uint64_t cnt, void *buffer);

int ext2_write(partition_t *part, char *path, uint64_t start, uint64_t cnt, void *buffer);

int ext2_mkdir(partition_t *part, char *parent, char *name, uint16_t permissions);

int ext2_touch(partition_t *part, char *parent, char *name, uint16_t permissions);

void ext2_delete(partition_t *part, char *path);

#endif
