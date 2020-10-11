#pragma once

#include <kernel/fs/ext2/types.h>

#include <stdint.h>

namespace ext2 {

struct superblock_t {
    void writeBack(uint8_t partitionIndex);

    void read(uint8_t partitionIndex);

    superBlockData_t data;

    uint32_t blockSize;

    uint32_t fragmentSize;
};

inline superblock_t superblock;

}
