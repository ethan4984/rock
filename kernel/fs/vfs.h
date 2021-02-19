#ifndef VFS_H_
#define VFS_H_

#include <fs/device.h>
#include <types.h>

struct stat {
    dev_t st_dev;
    ino_t st_ino;
    mode_t st_mode;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    off_t st_size;
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
    blksize_t st_blksize;
    blkcnt_t st_blocks;
};

struct vfs_node {
    char *absolute_path;
    char *relative_path;
    char *name;

    struct filesystem *fs;
    struct stat stat;

    uninit_vec(struct vfs_node, child_nodes);
    struct vfs_node *parent;
};

extern struct vfs_node vfs_root_node;

struct vfs_node *vfs_create_node(struct vfs_node *parent, char *name);
struct vfs_node *vfs_create_node_deep(char *path);
struct vfs_node *vfs_relative_path(struct vfs_node *parent, char *name);
struct vfs_node *vfs_absolute_path(char *path);
struct vfs_node *vfs_check_node(struct vfs_node *node);
struct vfs_node *vfs_mkdir(struct vfs_node *parent, char *name);
int vfs_open(char *path, int flags);
int vfs_touch(char *path, uint16_t perms);
int vfs_mount_fs(struct filesystem *fs);
int vfs_mount_dev(char *dev, char *mount_gate);
int vfs_write(struct vfs_node *node, off_t off, off_t cnt, void *buf);
int vfs_read(struct vfs_node *node, off_t off, off_t cnt, void *buf);
int vfs_unlink(char *path);
int vfs_destory_node(char *absolute_path);

#endif
