#ifndef VFAT_FAT_H
#define VFAT_FAT_H

#include <stdint.h>
#include <stdbool.h>
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
    u32 *entries;
    u32 last_alloc_cluster;
};

void fat_create(struct vbr *vbr, struct fat *fat);
void fat_read(struct fdisk *disk, struct vbr *vbr, struct fat *fat);
void fat_write(struct fat *fat, struct fdisk *disk);

bool fat_alloc_chain(/*in*/ struct fat *fat, /*in*/ u32 length, /*out*/ u32 *start_cluster);
void fat_append_chain(/*in*/ struct fat *fat, /*in*/ u32 start_cluster, /*in*/ u32 new_cluster);
u32 fat_getchainlen(struct fat *fat, u32 start_cluster);
void fat_getchain(struct fat *fat, u32 start_cluster, u32 *chain);
void fat_seteof(struct fat* fat, u32 cluster);
void fat_setfree(struct fat* fat, u32 cluster);
u32 fat_get_free_cluster_count(struct fat *fat);

void fat_destruct(struct fat *fat);

#endif /* VFAT_FAT_H */
