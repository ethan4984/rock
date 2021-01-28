#ifndef VFS_H_
#define VFS_H_

#include <fs/device.h>
#include <types.h>

typedef struct {
    dev_t st_dev;
    ino_t st_ino;
    mode_t st_mode;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    off_t st_size;
    timespec_t st_atim;
    timespec_t st_mtim;
    timespec_t st_ctim;
    blksize_t st_blksize;
    blkcnt_t st_blocks;
} stat_t;

typedef struct vfs_node_t {
    char *absolute_path;
    char *relative_path;
    char *name;

    filesystem_t *fs;
    stat_t stat;

    uninit_vec(struct vfs_node_t, child_nodes);
    struct vfs_node_t *parent;
} vfs_node_t;

extern vfs_node_t vfs_root_node;

vfs_node_t *vfs_create_node(vfs_node_t *parent, char *name);
vfs_node_t *vfs_create_node_deep(vfs_node_t *parent, char *path);
vfs_node_t *vfs_relative_path(vfs_node_t *parent, char *name);
vfs_node_t *vfs_absolute_path(char *path);
vfs_node_t *vfs_check_node(vfs_node_t *node);
vfs_node_t *vfs_mkdir(vfs_node_t *parent, char *name);
vfs_node_t *vfs_find_parent(char *path);
int vfs_open(char *path, int flags);
int vfs_touch(char *path, uint16_t perms);
int vfs_mount_fs(filesystem_t *fs);
int vfs_mount_dev(char *dev, char *mount_gate);
int vfs_write(vfs_node_t *node, off_t off, off_t cnt, void *buf);
int vfs_read(vfs_node_t *node, off_t off, off_t cnt, void *buf);
int vfs_unlink(char *path);
int vfs_destory_node(char *absolute_path);

#endif
