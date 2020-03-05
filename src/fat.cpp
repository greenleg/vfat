#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/common.h"
#include "../include/fat.h"

static bool fat_alloc_cluster(/*in*/ struct fat *fat, /*out*/ uint32_t *cluster)
{
    uint32_t clus;

    for (clus = fat->last_alloc_cluster + 1; clus < fat->bootSector->GetClusterCount(); ++clus) {
        if (fat->entries[clus] == FAT_FREE) {
            fat->entries[clus] = FAT_EOF;
            fat->last_alloc_cluster = clus;
            *cluster = clus;
            return true;
        }
    }

    for (clus = FAT_FIRST_CLUSTER; clus < fat->last_alloc_cluster; ++clus) {
        if (fat->entries[clus] == FAT_FREE) {
            fat->entries[clus] = FAT_EOF;
            fat->last_alloc_cluster = clus;
            *cluster = clus;
            return true;
        }
    }

    __vfat_errno = EFATFULL;
    return false;
}

bool fat_alloc_chain(/*in*/ struct fat *fat, /*in*/ uint32_t length, /*out*/ uint32_t *start_cluster)
{
    uint32_t cluster, new_cluster;
    uint32_t i;

    if (!fat_alloc_cluster(fat, start_cluster)) {
        // vfat_errno contains an error code
        return false;
    }

    cluster = *start_cluster;
    for (i = 1; i < length; ++i) {        
        if (!fat_alloc_cluster(fat, &new_cluster)) {
            // vfat_errno contains an error code
            return false;
        }

        fat->entries[cluster] = new_cluster;
        cluster = new_cluster;
    }

    return true;
}

void fat_append_chain(/*in*/ struct fat *fat, /*in*/ uint32_t start_cluster, /*in*/ uint32_t new_cluster)
{
    uint32_t cluster = start_cluster;

    while (fat->entries[cluster] != FAT_EOF) {
        cluster = fat->entries[cluster];
    }

    fat->entries[cluster] = new_cluster;
}

uint32_t fat_getchainlen(struct fat *fat, uint32_t start_cluster)
{
    uint32_t cluster = start_cluster;
    uint32_t len = 1;

    while (fat->entries[cluster] != FAT_EOF) {
        cluster = fat->entries[cluster];
        ++len;
    }

    return len;
}

void fat_getchain(struct fat *fat, uint32_t start_cluster, uint32_t *chain)
{
    uint32_t cluster = start_cluster;
    int i = 0;

    chain[0] = start_cluster;
    while (fat->entries[cluster] != FAT_EOF) {
        cluster = fat->entries[cluster];
        chain[++i] = cluster;
    }
}

void fat_seteof(struct fat* fat, uint32_t cluster)
{
    fat->entries[cluster] = FAT_EOF;
}

void fat_setfree(struct fat* fat, uint32_t cluster)
{
    fat->entries[cluster] = FAT_FREE;
}

void fat_create(BootSector *bootSector, struct fat *fat)
{
    fat->bootSector = bootSector;
    fat->last_alloc_cluster = FAT_FIRST_CLUSTER - 1;
    fat->entries = static_cast<uint32_t *>(malloc(sizeof(uint32_t) * bootSector->GetClusterCount()));
    memset((void*)fat->entries, 0, sizeof(uint32_t) * bootSector->GetClusterCount());
    fat->entries[0] = FAT_MEDIA_DESCRIPTOR;
    fat->entries[1] = FAT_EOF;
}

void fat_read(org::vfat::FileDisk *device, BootSector *bootSector, struct fat *fat)
{
    uint32_t fat_dev_offset = bootSector->GetFatOffset();

    fat->bootSector = bootSector;
    fat->last_alloc_cluster = FAT_FIRST_CLUSTER - 1;
    fat->entries = static_cast<uint32_t *>(malloc(sizeof(uint32_t) * bootSector->GetClusterCount()));
    device->Read((uint8_t *)fat->entries, fat_dev_offset, sizeof(uint32_t) * bootSector->GetClusterCount());
}

void fat_write(struct fat *fat, org::vfat::FileDisk *device)
{
    BootSector *bootSector = fat->bootSector;
    uint32_t fat_dev_offset = bootSector->GetFatOffset();
    device->Write((uint8_t* )fat->entries, fat_dev_offset, sizeof(uint32_t) * bootSector->GetClusterCount());
}

void fat_destruct(struct fat *fat)
{
    free(fat->entries);
}

uint32_t fat_get_free_cluster_count(struct fat *fat)
{
    uint32_t clus;
    uint32_t cnt = 0;

    for (clus = FAT_FIRST_CLUSTER; clus < fat->bootSector->GetClusterCount(); ++clus) {
        if (fat->entries[clus] == FAT_FREE) {
            ++cnt;
        }
    }

    return cnt;
}
