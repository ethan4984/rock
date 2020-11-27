#ifndef FAT32_TYPES_H_
#define FAT32_TYPES_H_

#include <fs/vfs.h>

#define FAT32_FSINFO_SIGNATURE0 0x41615252
#define FAT32_FSINFO_SIGNATURE1 0x61417272
#define FAT32_FSINFO_SIGNATURE2 0xAA550000

#define FAT32_BPB_SIGNATURE0 0x28
#define FAT32_BPB_SIGNATURE1 0x29

#define FAT32_SYSTEM_IDENTIFIER "FAT32 "

typedef struct {
    uint8_t jmp[3];
    char oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_cnt;
    uint16_t dir_entry_cnt;
    uint16_t total_sectors;
    uint8_t media_desc_type;
    uint16_t legacy_sector_cnt;
    uint16_t sectors_per_track;
    uint16_t head_cnt;
    uint32_t hidden_sector_cnt;
    uint32_t large_sector_cnt;

    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t fat_version;
    uint32_t root_dir_cluster;
    uint16_t fsinfo_loc;
    uint16_t backup_bootsector;
    uint8_t reserved0[12];
    uint8_t drive_number;
    uint8_t reserved1;
    uint8_t signature;
    uint32_t volume_id;
    char volume_label[11];
    char system_identifier[8];
} __attribute__((packed)) fat32_bpb_t;

typedef struct {
    uint32_t signature0;
    uint8_t reserved0[480];
    uint32_t signature1;
    uint32_t last_free_cluster;
    uint32_t cluster_begin; 
    uint8_t reserved1[12];
    uint32_t signature2;
} __attribute__((packed)) fat32_fsinfo_t;

#endif
