#ifndef VFS_HPP_
#define VFS_HPP_

#include <string.hpp>
#include <map.hpp>
#include <cpu.hpp>

namespace vfs {

struct default_ioctl { 
    virtual int call([[maybe_unused]] regs *regs_cur) { 
        return -1; 
    }
};

class node;
class cluster;

struct fs {
    explicit fs() = default;

    int read(node *vfs_node, off_t off, off_t cnt, void *buf);
    int write(node *vfs_node, off_t off, off_t cnt, void *buf);
    int mkdir(node *vfs_node);
    int open(node *vfs_node, uint16_t status);
    int unlink(node *vfs_node);

    virtual int raw_read([[maybe_unused]] node *vfs_node, [[maybe_unused]] off_t off, [[maybe_unused]] off_t cnt, [[maybe_unused]] void *buf) {
        return -1;
    }

    virtual int raw_write([[maybe_unused]] node *vfs_node, [[maybe_unused]] off_t off, [[maybe_unused]] off_t cnt, [[maybe_unused]] void *buf) {
        return -1;
    }

    virtual int raw_mkdir([[maybe_unused]] node *vfs_node) {
        return -1;
    }

    virtual int raw_open([[maybe_unused]] node *vfs_node, [[maybe_unused]] uint16_t status) {
        return -1;
    }

    virtual int raw_unlink([[maybe_unused]] node *vfs_node) {
        return -1;
    }

    virtual int refresh([[maybe_unused]] node *vfs_node) {
        return -1;
    }

    cluster *root_cluster;
};

struct node {
    node(cluster *parent_cluster, fs *filesystem, uint16_t mode, lib::string absolute_path, default_ioctl *ioctl_device = NULL); 
    node() = default;

    int ioctl(regs *regs_cur);
    node *search_relative(lib::string name);

    lib::string name;

    stat *stat_cur;

    constexpr bool is_socket() const { return stat_cur->st_mode & s_ifsock; }
    constexpr bool is_link() const { return stat_cur->st_mode & s_iflnk; }
    constexpr bool is_reg() const { return stat_cur->st_mode & s_ifreg; }
    constexpr bool is_block() const { return stat_cur->st_mode & s_ifblk; }
    constexpr bool is_directory() const { return (stat_cur->st_mode & s_ifdir) ? true : false; }
    constexpr bool is_character_device() const { return stat_cur->st_mode & s_ifchr; }
    constexpr bool is_fifo() const { return stat_cur->st_mode & s_ififo; }

    void set_cluster(cluster *new_cluster) {
        child_cluster = new_cluster;
    }

    void remove_cluster() {
        child_cluster = NULL;
    }

    fs *filesystem;

    node *next; 
    node *last; 

    node *parent;
    node *child;

    default_ioctl *ioctl_device;

    cluster *parent_cluster;
    cluster *child_cluster;
};

struct cluster {
    cluster(node *vfs_node, fs *filesystem);
    cluster(fs *filesystem);
    cluster() = default; 

    node *search_absolute(lib::string absolute_path, node *vfs_node = NULL);
    void generate_node(lib::string absolute_path, fs *filesystem, uint16_t mode, default_ioctl *ioctl_device = NULL);

    node *root_node;
    fs *filesystem;
};

int mount(lib::string source, lib::string target);

lib::string get_absolute_path(node *vfs_node);
lib::string get_relative_path(node *vfs_node);

inline cluster *root_cluster = NULL;

}

#endif
