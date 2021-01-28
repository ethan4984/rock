#ifndef DEVICE_H_
#define DEVICE_H_

#include <vec.h>
#include <fs/ext2/types.h>
#include <types.h>

#define SECTOR_SIZE 0x200
#define DEVFS_PERMS 0x555
#define DEV_NAME_MAX 32

typedef struct {
    uint8_t drive_status;
    uint8_t starting_chs[3];
    uint8_t partition_type;
    uint8_t ending_chs[3];
    uint32_t starting_lba;
    uint32_t sector_cnt;
} __attribute__((packed)) mbr_partition_t;

typedef struct {
    uint64_t identifier;
    uint32_t version;
    uint32_t hdr_size;
    uint32_t checksum;
    uint32_t reserved0;
    uint64_t hdr_lba;
    uint64_t alt_hdr_lba;
    uint64_t first_block;
    uint64_t last_block;
    uint64_t guid[2];
    uint64_t guid_lba;
    uint32_t partition_ent_cnt;
    uint32_t partition_ent_size;
    uint32_t crc32_partition_array;
} __attribute__((packed)) gpt_partition_table_hdr;

struct filesystem_t;

typedef struct {
    struct filesystem_t *fs;
    size_t device_index;
    size_t partition_cnt;
    int (*read)(int, uint64_t, uint64_t, void*);
    int (*write)(int, uint64_t, uint64_t, void*);
} msd_t;

typedef struct {
    size_t devfs_id;
    char *dev_name;
    size_t partition_offset;
    msd_t *device;
} devfs_node_t;

struct vfs_node_t;

typedef struct filesystem_t {
    devfs_node_t *devfs_node;

    ext2_fs_t *ext2_fs;
    char *mount_gate;

    int (*read)(struct vfs_node_t*, off_t, off_t, void*);
    int (*write)(struct vfs_node_t*, off_t, off_t, void*);
    int (*mkdir)(struct vfs_node_t*, uint16_t);
    int (*refresh)(struct vfs_node_t*);
    int (*open)(struct vfs_node_t*, int flags);
    int (*unlink)(struct vfs_node_t*);
} filesystem_t;

void msd_raw_read(devfs_node_t *dev, uint64_t start, uint64_t cnt, void *ret);
void msd_raw_write(devfs_node_t *dev, uint64_t start, uint64_t cnt, void *ret);
void scan_device_partitions(msd_t *device);
void init_devfs();
int devfs_add_device(devfs_node_t node);
devfs_node_t *path2devfs(char *path);

extern_vec(msd_t, msd_list);
extern_vec(filesystem_t, filesystem_list);

#endif
