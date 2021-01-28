#ifndef TYPES_H_
#define TYPES_H_

#define SEEK_SET 1
#define SEEK_CUR 2
#define SEEK_END 3

#define O_ACCMODE 0x0007
#define O_EXEC    1
#define O_RDONLY  2
#define O_RDWR    3
#define O_SEARCH  4
#define O_WRONLY  5

#define O_APPEND    0x0008
#define O_CREAT     0x0010
#define O_DIRECTORY 0x0020
#define O_EXCL      0x0040
#define O_NOCTTY    0x0080
#define O_NOFOLLOW  0x0100
#define O_TRUNC     0x0200
#define O_NONBLOCK  0x0400
#define O_DSYNC     0x0800
#define O_RSYNC     0x1000
#define O_SYNC      0x2000
#define O_CLOEXEC   0x4000

typedef int64_t ssize_t;
typedef int64_t off_t;

typedef uint64_t dev_t;
typedef uint64_t ino_t;
typedef int32_t mode_t;
typedef int32_t nlink_t;
typedef int64_t blksize_t;
typedef int64_t blkcnt_t;

typedef int32_t pid_t;
typedef int32_t tid_t;
typedef int32_t uid_t;
typedef int32_t gid_t;

typedef int64_t time_t;
typedef int64_t clockid_t;

typedef struct {
    time_t tv_sec;
    long tv_nsec;
} timespec_t;

#endif
