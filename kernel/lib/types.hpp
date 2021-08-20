#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <cstdint>
#include <cstddef>

using ssize_t = int64_t;
using off_t = int64_t;
using symbol = void*[]; 

using pid_t = ssize_t;
using tid_t = ssize_t;
using uid_t = uint32_t;
using gid_t = uint32_t;

using dev_t = uint64_t;
using ino_t =  uint64_t;
using mode_t = int32_t;
using nlink_t = int32_t;
using blksize_t = int64_t;
using blkcnt_t = int64_t;

using time_t = int64_t;
using clockid_t = int64_t;

constexpr size_t s_ifmt = 0x0f000;
constexpr size_t s_ifblk = 0x06000;
constexpr size_t s_ifchr = 0x02000;
constexpr size_t s_ififo = 0x01000;
constexpr size_t s_ifreg = 0x08000;
constexpr size_t s_ifdir = 0x04000;
constexpr size_t s_iflnk = 0x0a000;
constexpr size_t s_ifsock = 0x0c000;

constexpr size_t s_irwxu = 0700;
constexpr size_t s_irusr = 0400;
constexpr size_t s_iwusr = 0200;
constexpr size_t s_ixusr = 0100;
constexpr size_t s_irxwg = 070;
constexpr size_t s_irgrp = 040;
constexpr size_t s_iwgrp = 020;
constexpr size_t s_ixgrp = 010;
constexpr size_t s_irwxo = 07;
constexpr size_t s_iroth = 04;
constexpr size_t s_iwoth = 02;
constexpr size_t s_ixoth = 01;
constexpr size_t s_isuid = 04000;
constexpr size_t s_isgid = 02000;
constexpr size_t s_isvtx = 01000;

constexpr size_t s_iread = s_irusr;
constexpr size_t s_iwrite = s_iwusr;
constexpr size_t s_iexec = s_ixusr;

constexpr size_t dt_unknown = 0;
constexpr size_t dt_fifo = 1;
constexpr size_t dt_chr = 2;
constexpr size_t dt_dir = 4;
constexpr size_t dt_blk = 6;
constexpr size_t dt_reg = 8;
constexpr size_t dt_lnk = 10;
constexpr size_t dt_sock = 12;
constexpr size_t dt_wht = 14;

constexpr size_t seek_cur = 1;
constexpr size_t seek_end = 2;
constexpr size_t seek_set = 3;

constexpr size_t o_accmode = 0x7;
constexpr size_t o_exec = 0x1;
constexpr size_t o_rdonly = 0x2;
constexpr size_t o_rdwr = 0x3;
constexpr size_t o_search = 0x4;
constexpr size_t o_wronly = 0x5;

constexpr size_t o_append = 0x8;
constexpr size_t o_creat = 0x10;
constexpr size_t o_directory = 0x20;
constexpr size_t o_excl = 0x40;
constexpr size_t o_noctty = 0x80;
constexpr size_t o_nofollow = 0x100;
constexpr size_t o_trunc = 0x200;
constexpr size_t o_nonblock = 0x400;
constexpr size_t o_dsync = 0x800;
constexpr size_t o_rsync = 0x1000;
constexpr size_t o_sync = 0x2000;
constexpr size_t o_cloexec = 0x4000;

constexpr size_t ebadf = 1008;
constexpr size_t enoent = 1043;
constexpr size_t einval = 1026;
constexpr size_t enotty = 1058;
constexpr size_t eexist = 1019;
constexpr size_t eisdir = 1019;
constexpr size_t espipe = 1069;
constexpr size_t enotdir = 1053;

constexpr size_t f_dupfd = 1;
constexpr size_t f_dupfd_cloexec = 2;
constexpr size_t f_getfd = 3;
constexpr size_t f_setfd = 4;
constexpr size_t f_getfl = 5;
constexpr size_t f_setfl = 6;
constexpr size_t f_getlk = 7;
constexpr size_t f_setlk = 8;
constexpr size_t f_setlkw = 9;
constexpr size_t f_getown = 10;
constexpr size_t f_setown = 11; 

constexpr size_t f_rdlck = 1;
constexpr size_t f_unlck = 2;
constexpr size_t f_wrlck = 3;

constexpr size_t fd_cloexec = 1;
constexpr int32_t at_fdcwd = -100;

constexpr size_t wifexited = 0x200;

struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

struct stat {
    dev_t st_dev;
    ino_t st_ino;
    mode_t st_mode;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    off_t st_size;
    timespec st_atim;
    timespec st_mtim;
    timespec st_ctim;
    blksize_t st_blksize;
    blkcnt_t st_blocks;
};

struct dirent {
    ino_t d_ino;
    off_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[1024];
};

#endif
