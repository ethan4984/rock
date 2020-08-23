#pragma once

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

struct partition {
    uint8_t fsType;
    mbrPartitionEntry mbr;
};

enum {
    NOFS,
    EXT2
};

void readPartitions();

inline partition *partitions = NULL;

}
