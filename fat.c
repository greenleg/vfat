#include <stdlib.h>
#include <string.h>
#include "fat.h"

static uint32_t fat_alloc_cluster(struct fat *fat)
{
    uint32_t cluster;

    for (cluster = fat->last_alloc_cluster + 1; cluster < fat->vbr->cluster_count; ++cluster) {
        if (fat->entries[cluster] == FAT_FREE) {
            fat->entries[cluster] = FAT_EOF;
            fat->last_alloc_cluster = cluster;
            return cluster;
        }
    }

    for (cluster = FAT_FIRST_CLUSTER; cluster < fat->last_alloc_cluster; ++cluster) {
        if (fat->entries[cluster] == FAT_FREE) {
            fat->entries[cluster] = FAT_EOF;
            fat->last_alloc_cluster = cluster;
            return cluster;
        }
    }

    /* TODO: Error handling with appropriate status, e.g. E_FAT_FULL */
    return 0;
}

uint32_t fat_alloc_chain(struct fat *fat, uint32_t length)
{
    const uint32_t start_cluster = fat_alloc_cluster(fat);
    uint32_t cluster = start_cluster;
    uint32_t new_cluster;
    uint32_t i;

    for (i = 1; i < length; ++i) {
        new_cluster = fat_alloc_cluster(fat);
        fat->entries[cluster] = new_cluster;
        cluster = new_cluster;
    }

    return start_cluster;
}

void fat_append_to_chain(struct fat *fat, uint32_t start_cluster, uint32_t new_cluster)
{
    uint32_t cluster = start_cluster;

    while (fat->entries[cluster] != FAT_EOF) {
        cluster = fat->entries[cluster];
    }

    fat->entries[cluster] = new_cluster;
}

uint32_t fat_get_chain_length(struct fat *fat, uint32_t start_cluster)
{
    uint32_t cluster = start_cluster;
    int len = 1;

    while (fat->entries[cluster] != FAT_EOF) {
        cluster = fat->entries[cluster];
        ++len;
    }

    return len;
}

void fat_get_chain(struct fat *fat, uint32_t start_cluster, uint8_t *chain)
{
    uint32_t cluster = start_cluster;
    int i = 0;

    chain[0] = start_cluster;
    while (fat->entries[cluster] != FAT_EOF) {
        cluster = fat->entries[cluster];
        chain[++i] = cluster;
    }
}

void fat_set_eof(struct fat* fat, uint32_t cluster)
{
    fat->entries[cluster] = FAT_EOF;
}

void fat_set_free(struct fat* fat, uint32_t cluster)
{
    fat->entries[cluster] = FAT_FREE;
}

void fat_create(struct vbr *vbr, struct fat *fat)
{
    fat->vbr = vbr;
    fat->last_alloc_cluster = FAT_FIRST_CLUSTER - 1;
    fat->entries = malloc(sizeof(uint32_t) * vbr->cluster_count);
    memset((void*)fat->entries, 0, sizeof(uint32_t) * vbr->cluster_count);
    fat->entries[0] = FAT_MEDIA_DESCRIPTOR;
    fat->entries[1] = FAT_EOF;
}

void fat_read(struct fdisk *disk, struct vbr *vbr, struct fat *fat)
{
    uint32_t fat_offset_in_bytes = vbr->fat_offset * vbr_get_bytes_per_sector(vbr);

    fat->vbr = vbr;
    fat->last_alloc_cluster = FAT_FIRST_CLUSTER - 1;
    fdisk_read(disk, (uint8_t*)fat->entries, fat_offset_in_bytes, sizeof(uint32_t) * vbr->cluster_count);
}

void fat_write(struct fat *fat, struct fdisk *disk)
{
    struct vbr *vbr = fat->vbr;
    uint32_t fat_offset_in_bytes = vbr->fat_offset * vbr_get_bytes_per_sector(vbr);

    fdisk_write(disk, (uint8_t*)fat->entries, fat_offset_in_bytes, sizeof(uint32_t) * vbr->cluster_count);
}

void fat_destruct(struct fat *fat)
{
    free(fat->entries);
}

uint32_t fat_get_free_clusters(struct fat *fat)
{
    uint32_t cluster;
    uint32_t cnt = 0;

    for (cluster = FAT_FIRST_CLUSTER; cluster < fat->vbr->cluster_count; ++cluster) {
        if (fat->entries[cluster] == FAT_FREE) {
            ++cnt;
        }
    }

    return cnt;
}
