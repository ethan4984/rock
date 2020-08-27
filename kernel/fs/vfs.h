#pragma once

#include <lib/memoryUtils.h>

#include <stdint.h>
#include <stddef.h>

namespace kernel {

struct mbrPartitionEntry {
    uint8_t bootIndicator;
    uint8_t startingCHS[3]; 
    uint8_t systemID;
    uint8_t endingCHS[3];
    uint32_t startingSector;
    uint32_t totalSize;
} __attribute__((packed));

struct partition_t {
    uint8_t fsType;
    mbrPartitionEntry mbr;
};

enum {
    NOFS,
    EXT2
};

class fs {
public:
    function<int, int, void *, uint64_t> read; 
    function<int, int, const void *, uint64_t> write;
    function<int, int> close;
};

void readPartitions();

inline partition_t *partitions = NULL;

inline fs *fsFunctions;

}
