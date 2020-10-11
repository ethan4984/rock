#pragma once

#include <lib/memoryUtils.h>

#include <stdint.h>
#include <stddef.h>

struct [[gnu::packed]] mbrPartitionEntry {
    uint8_t bootIndicator;
    uint8_t startingCHS[3]; 
    uint8_t systemID;
    uint8_t endingCHS[3];
    uint32_t startingSector;
    uint32_t totalSize;
};

struct fd {
    char *path;
    int partition, fdID = -1;
    size_t index;
    int flags; 

    function<size_t, const char *, uint64_t, uint64_t, void *, int> read;
    function<size_t, char *, uint64_t, uint64_t, void *, int> write;
};

struct partition_t {
    uint8_t fsType;
    mbrPartitionEntry mbr;
};

enum {
    NOFS,
    EXT2,
    FAT32,
    ECHFS
};

namespace vfs {

size_t read(uint32_t fd, char *buf, size_t cnt);
size_t write(uint32_t fd, char *buf, size_t cnt);

int open(const char *path, int flags, int mode);
int close(int fd);

void readPartitions();

}

inline partition_t *partitions = NULL;
inline size_t partitionCnt = 0;
