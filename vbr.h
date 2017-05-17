#ifndef VFAT_VBR_H
#define VFAT_VBR_H

#include <stdint.h>
#include "fdisk.h"

#define VBR_SIZE 512

struct vbr
{
    /* Size of total volume in sectors */
    uint64_t volume_length;

    /* Sector address of 1st FAT */
    uint32_t fat_offset;

    /* Size of FAT in Sectors */
    uint32_t fat_length;

    /* Sector address of the Data Region */
    uint32_t cluster_heap_offset;

    /* Number of clusters in the Cluster Heap */
    uint32_t cluster_count;

    /* Cluster address of the Root Directory */
    uint32_t root_dir_first_cluster;

    /* This is a power of 2. Range: min of 29 = 512 byte cluster size, and a max of 212 = 4096. */
    uint8_t bytes_per_sector;

    /*
     * This is a power of 2. Range: Min of 21=512. The maximum Cluster size is 32 MiB,
     * so the Values in Bytes per Sector + Sectors Per Cluster cannot exceed 25.
     */
    uint8_t sectors_per_cluster;
};

void vbr_read(struct fdisk *disk, struct vbr *vbr);
void vbr_write(struct vbr *vbr, struct fdisk *disk);
void vbr_create(struct vbr *vbr, uint64_t volume_size, uint16_t bytes_per_sector, uint16_t sector_per_cluster);

uint16_t vbr_get_bytes_per_sector(struct vbr *vbr);
void     vbr_set_bytes_per_sector(struct vbr *vbr, uint16_t val);
uint16_t vbr_get_sectors_per_cluster(struct vbr *vbr);
void     vbr_set_sectors_per_cluster(struct vbr *vbr, uint16_t val);

#endif /* VFAT_VBR_H */
