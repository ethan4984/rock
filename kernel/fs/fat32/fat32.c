#include <fs/fat32/fat32.h>
#include <memutils.h>

static fat32_bpb_t bpb;
static fat32_fsinfo_t fsinfo;

int check_fs_fat32(partition_t *part) { 
    partition_read(part, 0, sizeof(fat32_bpb_t), &bpb);

    if((bpb.signature != FAT32_BPB_SIGNATURE0) && (bpb.signature != FAT32_BPB_SIGNATURE1)) 
        return 0;

    if(strncmp(bpb.system_identifier, FAT32_SYSTEM_IDENTIFIER, strlen(FAT32_SYSTEM_IDENTIFIER)) != 0)
        return 0;

    part->sector_size = bpb.bytes_per_sector; 

    partition_read(part, bpb.fsinfo_loc * part->sector_size, sizeof(fat32_fsinfo_t), &fsinfo);

    if((fsinfo.signature0 != FAT32_FSINFO_SIGNATURE0) || (fsinfo.signature1 != FAT32_FSINFO_SIGNATURE1) || (fsinfo.signature2 != FAT32_FSINFO_SIGNATURE2))
        return 0;

    return 1; 
}
