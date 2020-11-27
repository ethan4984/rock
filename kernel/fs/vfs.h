#ifndef VFS_H_
#define VFS_H_

#include <fs/ext2/types.h>

enum {
    EXT2,
    FAT32,
    UNKNOWN
};

typedef struct {
    uint64_t device_index;
    int (*read)(int, uint64_t, uint64_t, void*);
    int (*write)(int, uint64_t, uint64_t, void*);
} device_t;

typedef struct partition_t partition_t; 

struct partition_t {
    device_t *device;
    uint64_t device_offset, sector_size;
    int fs_type;
    ext2_fs_t *ext2_fs;

    int (*read)(partition_t*, char*, uint64_t, uint64_t, void*);
    int (*write)(partition_t*, char*, uint64_t, uint64_t, void*);
};

void partition_read(partition_t *partition, uint64_t start, uint64_t cnt, void *ret); 

void partition_write(partition_t *partition, uint64_t start, uint64_t cnt, void *ret); 

void add_device(device_t *device);

void vfs_init();

#endif
