#ifndef VFAT_FAT_H
#define VFAT_FAT_H

#include <stdint.h>
#include <stdbool.h>
#include "BootSector.h"

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

using namespace org::vfat;

struct fat
{
    BootSector *bootSector;
    uint32_t *entries;
    uint32_t last_alloc_cluster;
};

void fat_create(BootSector *bootSector, struct fat *fat);
void fat_read(org::vfat::FileDisk *device, BootSector *bootSector, struct fat *fat);
void fat_write(struct fat *fat, org::vfat::FileDisk *device);

bool fat_alloc_chain(/*in*/ struct fat *fat, /*in*/ uint32_t length, /*out*/ uint32_t *start_cluster);
void fat_append_chain(/*in*/ struct fat *fat, /*in*/ uint32_t start_cluster, /*in*/ uint32_t new_cluster);
uint32_t fat_getchainlen(struct fat *fat, uint32_t start_cluster);
void fat_getchain(struct fat *fat, uint32_t start_cluster, uint32_t *chain);
void fat_seteof(struct fat* fat, uint32_t cluster);
void fat_setfree(struct fat* fat, uint32_t cluster);
uint32_t fat_get_free_cluster_count(struct fat *fat);

void fat_destruct(struct fat *fat);

#endif /* VFAT_FAT_H */
