#ifndef VFAT_VBR_H
#define VFAT_VBR_H

#include <stdint.h>
#include "common.h"
#include "fdisk.h"

#define VBR_SIZE 512

struct vbr
{
    /* Size of total volume in sectors */
    u64 volume_length;

    /* Sector address of 1st FAT */
    u32 fat_offset;

    /* Size of FAT in Sectors */
    u32 fat_length;

    /* Sector address of the Data Region */
    u32 cluster_heap_offset;

    /* Number of clusters in the Cluster Heap */
    u32 cluster_count;

    /* Cluster address of the Root Directory */
    u32 rootdir_first_cluster;

    /* This is a power of 2. Range: min of 29 = 512 byte cluster size, and a max of 212 = 4096. */
    u8 bytes_per_sector_pow2;

    /*
     * This is a power of 2. Range: Min of 21=512. The maximum Cluster size is 32 MiB,
     * so the Values in Bytes per Sector + Sectors Per Cluster cannot exceed 25.
     */
    u8 sectors_per_cluster_pow2;
};

void vbr_read(struct fdisk *disk, struct vbr *vbr);
void vbr_write(struct vbr *vbr, struct fdisk *disk);
void vbr_create(struct vbr *vbr, u64 volume_size, u16 bytes_per_sector, u16 sector_per_cluster);

u16  vbr_get_bytes_per_sector(struct vbr *vbr);
void vbr_set_bytes_per_sector(struct vbr *vbr, u16 val);
u16  vbr_get_sectors_per_cluster(struct vbr *vbr);
void vbr_set_sectors_per_cluster(struct vbr *vbr, u16 val);
u32  vbr_get_bytes_per_cluster(struct vbr * vbr);

#endif /* VFAT_VBR_H */
