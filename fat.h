#ifndef VFAT_FAT_H
#define VFAT_FAT_H

#include <stdint.h>
#include "vbr.h"

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

#define FAT_MEDIA_DESCRIPTOR     0xFFFFFFF8
#define FAT_EOF                  0xFFFFFFFF
#define FAT_FREE                 0x00000000
#define FAT_FIRST_CLUSTER        2

struct fat
{
    struct vbr *vbr;
    uint32_t *entries;
    uint32_t last_alloc_cluster;
};

void fat_create(struct vbr *vbr, struct fat *fat);
void fat_read(struct fdisk *disk, struct vbr *vbr, struct fat *fat);
void fat_write(struct fat *fat, struct fdisk *disk);

uint32_t fat_alloc_chain(struct fat *fat, uint32_t length);
void fat_append_to_chain(struct fat *fat, uint32_t start_cluster, uint32_t new_cluster);
uint32_t fat_get_chain_length(struct fat *fat, uint32_t start_cluster);
void fat_get_chain(struct fat *fat, uint32_t start_cluster, uint8_t *chain);
void fat_set_eof(struct fat* fat, uint32_t cluster);
void fat_set_free(struct fat* fat, uint32_t cluster);
uint32_t fat_get_free_clusters(struct fat *fat);

void fat_destruct(struct fat *fat);

#endif /* VFAT_FAT_H */
