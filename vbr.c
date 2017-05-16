#include <stdint.h>
#include <sys/types.h>
#include <math.h>

#include "binaryreader.h"
#include "vbr.h"

#define VBR_SECTOR 0

#define VBR_VOLUMELENGTH_OFFSET 72
#define VBR_FATOFFSET_OFFSET 80
#define VBR_FATLENGTH_OFFSET 84
#define VBR_CLUSTERHEAPOFFSET_OFFSET 88
#define VBR_CLUSTERCOUNT_OFFSET 92
#define VBR_ROOTDIRFIRSTCLUSTER_OFFSET 96
#define VBR_BYTESPERSECTOR_OFFSET 108
#define VBR_SECTORSPERCLUSTER_OFFSET 109

static void vbr_readbuf(uint8_t *buf, struct vbr *vbr)
{
    vbr->volume_length = read_u64(buf, VBR_VOLUMELENGTH_OFFSET);
    vbr->fat_offset = read_u32(buf, VBR_FATOFFSET_OFFSET);
    vbr->fat_length = read_u32(buf, VBR_FATLENGTH_OFFSET);
    vbr->cluster_heap_offset = read_u32(buf, VBR_CLUSTERHEAPOFFSET_OFFSET);
    vbr->cluster_count = read_u32(buf, VBR_CLUSTERCOUNT_OFFSET);
    vbr->root_dir_first_cluster = read_u32(buf, VBR_ROOTDIRFIRSTCLUSTER_OFFSET);
    vbr->bytes_per_sector = read_u8(buf, VBR_BYTESPERSECTOR_OFFSET);
    vbr->sectors_per_cluster = read_u8(buf, VBR_SECTORSPERCLUSTER_OFFSET);
}

static void vbr_writebuf(struct vbr *vbr, uint8_t *buf)
{
    write_u64(buf, VBR_VOLUMELENGTH_OFFSET, vbr->volume_length);
    write_u32(buf, VBR_FATOFFSET_OFFSET, vbr->fat_offset);
    write_u32(buf, VBR_FATLENGTH_OFFSET, vbr->fat_length);
    write_u32(buf, VBR_CLUSTERHEAPOFFSET_OFFSET, vbr->cluster_heap_offset);
    write_u32(buf, VBR_CLUSTERCOUNT_OFFSET, vbr->cluster_count);
    write_u32(buf, VBR_ROOTDIRFIRSTCLUSTER_OFFSET, vbr->root_dir_first_cluster);
    write_u8(buf, VBR_BYTESPERSECTOR_OFFSET, vbr->bytes_per_sector);
    write_u8(buf, VBR_SECTORSPERCLUSTER_OFFSET, vbr->sectors_per_cluster);
}

void vbr_read(struct fdisk *disk, struct vbr *vbr)
{
    uint8_t buf[VBR_SIZE];
    fdisk_read(disk, buf, 0, VBR_SIZE);
    vbr_readbuf(buf, vbr);
}

void vbr_write(struct vbr *vbr, struct fdisk *disk)
{
    uint8_t buf[VBR_SIZE];
    vbr_writebuf(vbr, buf);
    fdisk_write(disk, buf, 0, VBR_SIZE);
}

void vbr_format(struct vbr *vbr, uint64_t volume_size, uint16_t bytes_per_sector, uint16_t sectors_per_cluster)
{
    vbr_set_bytes_per_sector(vbr, bytes_per_sector);
    vbr_set_sectors_per_cluster(vbr, sectors_per_cluster);

    /* Bytes Per Sector, Sectors Per Cluster should be a power of two */
    bytes_per_sector = vbr_get_bytes_per_sector(vbr);
    sectors_per_cluster = vbr_get_sectors_per_cluster(vbr);

    vbr->volume_length = volume_size / bytes_per_sector;
    vbr->fat_offset = 1;

    vbr->cluster_count = (uint32_t) ((vbr->volume_length - 1) / (4.0 / bytes_per_sector + sectors_per_cluster));
    vbr->fat_length = (uint32_t)ceil((4.0 * vbr->cluster_count) / bytes_per_sector);
    vbr->cluster_heap_offset = vbr->fat_offset + vbr->fat_length;
    vbr->root_dir_first_cluster = 2;
}

uint16_t vbr_get_bytes_per_sector(struct vbr *vbr)
{
    return 1 << (vbr->bytes_per_sector);
}

void vbr_set_bytes_per_sector(struct vbr *vbr, uint16_t val)
{
    vbr->bytes_per_sector = log2(val);
}

uint16_t vbr_get_sectors_per_cluster(struct vbr *vbr)
{
    return 1 << (vbr->sectors_per_cluster);
}

void vbr_set_sectors_per_cluster(struct vbr *vbr, uint16_t val)
{
    vbr->sectors_per_cluster = log2(val);
}
