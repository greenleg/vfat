#ifndef VFAT_FAT_H
#define VFAT_FAT_H

#include <stdint.h>

/*
 * There are a few special values that relate to the FAT:
 * 0x00000000 – No significant meaning
 * 0x00000001 – Not a valid cell value
 * 0xFFFFFFF6 – Largest Value
 * 0xFFFFFFF7 – Bad Block
 * 0xFFFFFFF8 – Media Descriptor
 * 0xFFFFFFF9-0xFFFFFFFE – Not Defined
 * 0xFFFFFFFF – End of File (EOF)
 */

#define FAT_NOT_VALID_CELL_VALUE 0x00000001
#define FAT_MEDIA_DESCRIPTOR     0xFFFFFFF8
#define FAT_EOF                  0xFFFFFFFF

struct fat
{
    struct vbr *vbr;
    uint8_t *entries;
    uint32_t count;
};

void fat_create(struct vbr *vbr, struct fat *fat);
void fat_read(struct fdisk *disk, struct vbr *vbr, struct fat *fat);
void fat_write(struct fat *fat, struct fdisk *disk);

uint32_t fat_alloc(uint32_t num_cluster);

#endif /* VFAT_FAT_H */
