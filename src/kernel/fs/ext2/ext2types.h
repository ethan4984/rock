#pragma once

#include <kernel/fs/ext2/ext2types.h>

#include <stdint.h>

struct inode_t {
    uint16_t permissions;
    uint16_t userID;

    uint32_t size32l;
    uint32_t accessTime;
    uint32_t creationTime;
    uint32_t modificationTime;
    uint32_t deletionTime;
    
    uint16_t groupID;
    uint16_t hardLinkCnt;

    uint32_t sectorCnt;
    uint32_t flags;
    uint32_t oss1;

    uint32_t blocks[15];

    uint32_t generationNumber;
    uint32_t eab;
    uint32_t size32h;

    uint32_t blockAddrFragment;

    /* ext2 linux */

    uint8_t fragNum;
    uint8_t fragSize;
    
    uint16_t reserved16;
    uint16_t userIDhigh;
    uint16_t groupIDhigh;

    uint32_t reserved32;
} __attribute__((packed));

struct fileEntry_t {
    int size;
    inode_t rootInode;
    inode_t inode;
};

struct blockGroupDescriptor_t {
    uint32_t blockAddressBitmap;
    uint32_t blockAddressInodeBitmap;
    uint32_t startingBlock;

    uint16_t unallocatedBlocks;
    uint16_t unallocatedInodes;
    uint16_t directoryCnt;

    uint16_t reserved[7];
} __attribute__((packed));

struct directoryEntry_t {
    uint32_t inode;
    uint16_t sizeofEntry;
    uint8_t nameLength;
    uint8_t typeIndicator;
    char name[];
} __attribute__((packed));

struct superBlockData_t {
    uint32_t inodeCount;
    uint32_t blockCount;
    uint32_t reservedBlocksCount;
    uint32_t freeBlocksCount;
    uint32_t freeInodesCount;
    uint32_t sbBlock;
    uint32_t blockSize;
    uint32_t fragSize;
    uint32_t blocksPerGroup;
    uint32_t fragsPerGroup;
    uint32_t inodesPerGroup;
    uint32_t lastMount;
    uint32_t lastWrite;
    uint16_t mountCount;
    uint16_t maxMount;
    uint16_t magicNum;
    uint16_t state;
    uint16_t errors;
    uint16_t minorVersion; 
    uint32_t lastCheck;
    uint32_t checkInterval;
    uint32_t osID;
    uint32_t majorVersion;
    uint16_t userID;
    uint16_t groupID;

    /* Extended Superblock */
    uint32_t firstInode;
    uint16_t inodeSize;
    uint16_t spBlockGroup;
    uint32_t featureCompat;
    uint32_t featureIncompat;
    uint32_t featureRCompat;
    uint64_t uuid[2];
    uint64_t volumeName[2];
    uint64_t lastMounted[8];
} __attribute__((packed));

