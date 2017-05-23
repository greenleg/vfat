#include <stdlib.h>
#include <string.h>
#include "fat.h"

static u32 fat_alloc_cluster(struct fat *fat)
{
    u32 clus;

    for (clus = fat->last_alloc_cluster + 1; clus < fat->vbr->cluster_count; ++clus) {
        if (fat->entries[clus] == FAT_FREE) {
            fat->entries[clus] = FAT_EOF;
            fat->last_alloc_cluster = clus;
            return clus;
        }
    }

    for (clus = FAT_FIRST_CLUSTER; clus < fat->last_alloc_cluster; ++clus) {
        if (fat->entries[clus] == FAT_FREE) {
            fat->entries[clus] = FAT_EOF;
            fat->last_alloc_cluster = clus;
            return clus;
        }
    }

    /* TODO: Error handling with appropriate status, e.g. E_FAT_FULL */
    return E_FAT_FULL;
}

u32 fat_alloc_chain(struct fat *fat, u32 length)
{
    const u32 start_cluster = fat_alloc_cluster(fat);
    u32 cluster = start_cluster;
    u32 new_cluster;
    u32 i;

    for (i = 1; i < length; ++i) {
        new_cluster = fat_alloc_cluster(fat);
        fat->entries[cluster] = new_cluster;
        cluster = new_cluster;
    }

    return start_cluster;
}

void fat_append_to_chain(struct fat *fat, u32 start_cluster, u32 new_cluster)
{
    uint32_t cluster = start_cluster;

    while (fat->entries[cluster] != FAT_EOF) {
        cluster = fat->entries[cluster];
    }

    fat->entries[cluster] = new_cluster;
}

u32 fat_getchainlen(struct fat *fat, u32 start_cluster)
{
    u32 cluster = start_cluster;
    int len = 1;

    /*if (fat->entries[cluster] == FAT_FREE) {
        return 0;
    }*/

    while (fat->entries[cluster] != FAT_EOF) {
        cluster = fat->entries[cluster];
        ++len;
    }

    return len;
}

void fat_getchain(struct fat *fat, u32 start_cluster, u32 *chain)
{
    u32 cluster = start_cluster;
    int i = 0;

    chain[0] = start_cluster;
    while (fat->entries[cluster] != FAT_EOF) {
        cluster = fat->entries[cluster];
        chain[++i] = cluster;
    }
}

void fat_seteof(struct fat* fat, u32 cluster)
{
    fat->entries[cluster] = FAT_EOF;
}

void fat_setfree(struct fat* fat, u32 cluster)
{
    fat->entries[cluster] = FAT_FREE;
}

void fat_create(struct vbr *vbr, struct fat *fat)
{
    fat->vbr = vbr;
    fat->last_alloc_cluster = FAT_FIRST_CLUSTER - 1;
    fat->entries = malloc(sizeof(u32) * vbr->cluster_count);
    memset((void*)fat->entries, 0, sizeof(uint32_t) * vbr->cluster_count);
    fat->entries[0] = FAT_MEDIA_DESCRIPTOR;
    fat->entries[1] = FAT_EOF;
}

void fat_read(struct fdisk *disk, struct vbr *vbr, struct fat *fat)
{
    u32 fat_offset_in_bytes = vbr->fat_offset * vbr_get_bytes_per_sector(vbr);

    fat->vbr = vbr;
    fat->last_alloc_cluster = FAT_FIRST_CLUSTER - 1;
    fat->entries = malloc(sizeof(u32) * vbr->cluster_count);
    fdisk_read(disk, (u8 *)fat->entries, fat_offset_in_bytes, sizeof(u32) * vbr->cluster_count);
}

void fat_write(struct fat *fat, struct fdisk *disk)
{
    struct vbr *vbr = fat->vbr;
    u32 fat_offset_in_bytes = vbr->fat_offset * vbr_get_bytes_per_sector(vbr);

    fdisk_write(disk, (u8*)fat->entries, fat_offset_in_bytes, sizeof(u32) * vbr->cluster_count);
}

void fat_destruct(struct fat *fat)
{
    free(fat->entries);
}

u32 fat_get_free_cluster_count(struct fat *fat)
{
    u32 clus;
    u32 cnt = 0;

    for (clus = FAT_FIRST_CLUSTER; clus < fat->vbr->cluster_count; ++clus) {
        if (fat->entries[clus] == FAT_FREE) {
            ++cnt;
        }
    }

    return cnt;
}
