#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <cstdint>
#include <cstddef>

using ssize_t = int64_t;
using off_t = int64_t;
using symbol = void*[]; 

using pid_t = ssize_t;
using tid_t = ssize_t;
using uid_t = ssize_t;
using gid_t = ssize_t;

using dev_t = uint64_t;
using ino_t =  uint64_t;
using mode_t = int32_t;
using nlink_t = int32_t;
using blksize_t = int64_t;
using blkcnt_t = int64_t;

using time_t = int64_t;
using clockid_t = int64_t;

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

#endif
