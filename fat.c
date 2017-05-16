#include "fat.h"

static uint32_t fat_alloc_cluster(struct fat *fat)
{
    const uint32_t last_cluster = fat->vbr->cluster_count + FAT_FIRST_CLUSTER;
    uint32_t cluster;

    for (cluster = fat->last_alloc_cluster + 1; cluster < last_cluster; ++cluster) {
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
