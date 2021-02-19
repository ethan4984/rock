#ifndef TYPES_H_
#define TYPES_H_

#include <stddef.h>
#include <stdint.h>

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 3

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

/* errno for mlibc */

#define EDOM 1
#define EILSEQ 2
#define ERANGE 3

#define E2BIG 1001
#define EACCES 1002
#define EADDRINUSE 1003
#define EADDRNOTAVAIL 1004
#define EAFNOSUPPORT 1005
#define EAGAIN 1006
#define EALREADY 1007
#define EBADF 1008
#define EBADMSG 1009
#define EBUSY 1010
#define ECANCELED 1011
#define ECHILD 1012
#define ECONNABORTED 1013
#define ECONNREFUSED 1014
#define ECONNRESET 1015
#define EDEADLK 1016
#define EDESTADDRREQ 1017
#define EDQUOT 1018
#define EEXIST 1019
#define EFAULT 1020
#define EFBIG 1021
#define EHOSTUNREACH 1022
#define EIDRM 1023
#define EINPROGRESS 1024
#define EINTR 1025
#define EINVAL 1026
#define EIO 1027
#define EISCONN 1028
#define EISDIR 1029
#define ELOOP 1030
#define EMFILE 1031
#define EMLINK 1032
#define EMSGSIZE 1034
#define EMULTIHOP 1035
#define ENAMETOOLONG 1036
#define ENETDOWN 1037
#define ENETRESET 1038
#define ENETUNREACH 1039
#define ENFILE 1040
#define ENOBUFS 1041
#define ENODEV 1042
#define ENOENT 1043
#define ENOEXEC 1044
#define ENOLCK 1045
#define ENOLINK 1046
#define ENOMEM 1047
#define ENOMSG 1048
#define ENOPROTOOPT 1049
#define ENOSPC 1050
#define ENOSYS 1051
#define ENOTCONN 1052
#define ENOTDIR 1053
#define ENOTEMPTY 1054
#define ENOTRECOVERABLE 1055
#define ENOTSOCK 1056
#define ENOTSUP 1057
#define ENOTTY 1058
#define ENXIO 1059
#define EOPNOTSUPP 1060
#define EOVERFLOW 1061
#define EOWNERDEAD 1062
#define EPERM 1063
#define EPIPE 1064
#define EPROTO 1065
#define EPROTONOSUPPORT 1066
#define EPROTOTYPE 1067
#define EROFS 1068
#define ESPIPE 1069
#define ESRCH 1070
#define ESTALE 1071
#define ETIMEDOUT 1072
#define ETXTBSY 1073
#define EWOULDBLOCK EAGAIN
#define EXDEV 1075
#define ENODATA 1076
#define ETIME 1077
#define ENOKEY 1078
#define ESHUTDOWN 1079
#define EHOSTDOWN 1080
#define EBADFD 1081
#define ENOMEDIUM 1082
#define ENOTBLK 1083

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

struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

#endif
