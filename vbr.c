#include <stdint.h>
#include <sys/types.h>
#include <math.h>

#include "sector.h"
#include "vbr.h"

#define VBR_SECTOR 0

#define VBR_FATOFFSET_OFFSET 80
#define VBR_FATLENGTH_OFFSET 84
#define VBR_CLUSTERHEAPOFFSET_OFFSET 88
#define VBR_CLUSTERCOUNT_OFFSET 92
#define VBR_ROOTDIRFIRSTCLUSTER_OFFSET 96
#define VBR_BYTESPERSECTOR_OFFSET 108
#define VBR_SECTORPERCLUSTER_OFFSET 109

/*static uint8_t log2(uint32_t x)
{
    uint8_t i = 0;
    uint32_t val = 1;
    while (val < x) {
        val <<= 1;
        ++i;
    }

    return i;
}*/

void vbr_read(struct sector *sector, struct vbr *vbr)
{
    vbr->fat_offset = sector_read_uint32(sector, VBR_FATOFFSET_OFFSET);
    vbr->fat_length = sector_read_uint32(sector, VBR_FATLENGTH_OFFSET);
    vbr->cluster_heap_offset = sector_read_uint32(sector, VBR_CLUSTERHEAPOFFSET_OFFSET);
    vbr->cluster_count = sector_read_uint32(sector, VBR_CLUSTERCOUNT_OFFSET);
    vbr->root_dir_first_cluster = sector_read_uint32(sector, VBR_ROOTDIRFIRSTCLUSTER_OFFSET);
    vbr->bytes_per_sector = sector_read_uint8(sector, VBR_BYTESPERSECTOR_OFFSET);
    vbr->sector_per_cluster = sector_read_uint8(sector, VBR_SECTORPERCLUSTER_OFFSET);
}

void vbr_write(struct vbr *vbr, struct sector *sector)
{
    sector_write_uint32(sector, VBR_FATOFFSET_OFFSET, vbr->fat_offset);
    sector_write_uint32(sector, VBR_FATLENGTH_OFFSET, vbr->fat_length);
    sector_write_uint32(sector, VBR_CLUSTERHEAPOFFSET_OFFSET, vbr->cluster_heap_offset);
    sector_write_uint32(sector, VBR_CLUSTERCOUNT_OFFSET, vbr->cluster_count);
    sector_write_uint32(sector, VBR_ROOTDIRFIRSTCLUSTER_OFFSET, vbr->root_dir_first_cluster);
    sector_write_uint8(sector, VBR_BYTESPERSECTOR_OFFSET, vbr->bytes_per_sector);
    sector_write_uint8(sector, VBR_SECTORPERCLUSTER_OFFSET, vbr->sector_per_cluster);
}

uint16_t vbr_get_bytes_per_sector(struct vbr *vbr)
{
    return 1 << (vbr->bytes_per_sector);
}

void vbr_set_bytes_per_sector(struct vbr *vbr, uint16_t val)
{
    vbr->bytes_per_sector = log2(val);
}

uint16_t vbr_get_sector_per_cluster(struct vbr *vbr)
{
    return 1 << (vbr->sector_per_cluster);
}

void vbr_set_sector_per_cluster(struct vbr *vbr, uint16_t val)
{
    vbr->sector_per_cluster = log2(val);
}

void vbr_format(struct vbr *vbr, uint64_t volume_size, uint16_t bytes_per_sector, uint16_t sector_per_cluster)
{
    vbr_set_bytes_per_sector(vbr, bytes_per_sector);
    vbr_set_sector_per_cluster(vbr, sector_per_cluster);

    bytes_per_sector = vbr_get_bytes_per_sector(vbr);
    sector_per_cluster = vbr_get_sector_per_cluster(vbr);

    vbr->volume_length = volume_size / bytes_per_sector;
    vbr->fat_offset = 1;

    vbr->cluster_count = (uint32_t) ((vbr->volume_length - 1) / (4.0 / bytes_per_sector + sector_per_cluster));
    vbr->fat_length = (uint32_t)ceil((4.0 * vbr->cluster_count) / bytes_per_sector);
    vbr->cluster_heap_offset = vbr->fat_offset + vbr->fat_length;
    vbr->root_dir_first_cluster = 2;
}

