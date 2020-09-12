#pragma once

#include <kernel/fs/ext2/ext2types.h>

#include <stdint.h>

namespace kernel {

class superblock_t {
public:
    void writeBack(uint8_t partitionIndex);

    void read(uint8_t partitionIndex);

    superBlockData_t data;

    uint32_t blockSize;

    uint32_t fragmentSize;
};

inline superblock_t superblock;

}
