#include <math.h>

#include "clusterchain.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

static uint64_t get_dev_offset(struct vbr *vbr, uint32_t cluster, uint32_t cluster_offset)
{
    uint32_t cluster_size = vbr_get_bytes_per_cluster(vbr);
    return vbr->cluster_heap_offset + cluster * cluster_size + cluster_offset;
}

uint64_t cluster_chain_get_length(struct cluster_chain *cc)
{
    if (cc->start_cluster == 0) {
        return 0;
    }

    return fat_get_chain_length(cc->fat, cc->start_cluster);
}


uint64_t cluster_chain_get_length_on_disk(struct cluster_chain *cc)
{
    if (cc->start_cluster == 0) {
        return 0;
    }

    uint32_t cluster_size = vbr_get_bytes_per_cluster(cc->fat->vbr);
    uint32_t len = fat_get_chain_length(cc->fat, cc->start_cluster);

    return len * cluster_size;
}

uint32_t cluster_chain_set_size(struct cluster_chain *cc, uint32_t size)
{
    uint32_t cluster_size = vbr_get_bytes_per_cluster(cc->fat->vbr);
    uint32_t nr_clusters = (size + cluster_size - 1) / cluster_size;
    cluster_chain_set_length(cc, nr_clusters);

    return cluster_size * nr_clusters;
}

void cluster_chain_create(struct cluster_chain *cc, struct fat *fat, uint32_t length)
{
    cc->fat = fat;
    cc->start_cluster = 0;
    cluster_chain_set_length(cc, length);
}

void cluster_chain_read_data(struct fdisk *disk, struct cluster_chain *cc, uint32_t offset, uint32_t nbytes, uint8_t *dst)
{
    if (cc->start_cluster == 0 && nbytes > 0) {
        /* Cannot read from empty cluster chain */
    }

    struct fat *fat = cc->fat;
    struct vbr *vbr = fat->vbr;
    uint32_t cluster_size = vbr_get_bytes_per_cluster(vbr);
    uint32_t chain_idx;
    uint32_t n;

    uint32_t chain[fat_get_chain_length(fat, cc->start_cluster)];
    fat_get_chain(fat, cc->start_cluster, chain);

    chain_idx = (offset / cluster_size);
    n = nbytes;

    if (offset % cluster_size != 0) {
        uint32_t cluster_offset = (offset % cluster_size);
        uint32_t size = MIN(cluster_size - cluster_offset, n);
        fdisk_read(disk, dst, get_dev_offset(vbr, chain[chain_idx], cluster_offset), size);
        dst += size;
        n -= size;
        ++chain_idx;
    }

    while (n > 0) {
        uint32_t size = MIN(cluster_size, n);
        fdisk_read(disk, dst, get_dev_offset(vbr, chain[chain_idx], 0), size);
        dst += size;
        n -= size;
        ++chain_idx;
    }
}

void cluster_chain_write_data(struct fdisk *disk, struct cluster_chain *cc, uint32_t offset, uint32_t nbytes, uint8_t *src)
{
    if (nbytes == 0) {
        return;
    }

    struct fat *fat = cc->fat;
    struct vbr *vbr = fat->vbr;
    uint32_t cluster_size = vbr_get_bytes_per_cluster(vbr);
    uint32_t min_size = offset + nbytes;
    uint32_t chain_idx;
    uint32_t n;
    uint32_t cluster_offset;
    uint32_t size;

    if (cluster_chain_get_length_on_disk(cc) < min_size) {
        cluster_chain_set_size(cc, min_size); // growing the chain
    }

    uint32_t chain[fat_get_chain_length(fat, cc->start_cluster)];
    fat_get_chain(fat, cc->start_cluster, chain);

    chain_idx = (offset / cluster_size);
    n = nbytes;

    if (offset % cluster_size != 0) {
        cluster_offset = (offset % cluster_size);
        size = MIN(cluster_size - cluster_offset, n);
        fdisk_write(disk, src, get_dev_offset(vbr, chain[chain_idx], cluster_offset), size);
        src += size;
        n -= size;
        ++chain_idx;
    }

    while (n > 0) {
        size = MIN(cluster_size, n);
        fdisk_write(disk, src, get_dev_offset(vbr, chain[chain_idx], 0), size);
        src += size;
        n -= size;
        ++chain_idx;
    }
}

void cluster_chain_set_length(struct cluster_chain *cc, uint32_t nr_clusters)
{
    if (cc->start_cluster == 0 && nr_clusters == 0) {
        /* nothing to do */
        return;
    }

    if (cc->start_cluster == 0 && nr_clusters > 0) {
        cc->start_cluster = fat_alloc_chain(cc->fat, nr_clusters);
        return;
    }

    uint32_t len = fat_get_chain_length(cc->fat, cc->start_cluster);
    if (nr_clusters == len) {
        return;
    }

    uint32_t chain[len];
    fat_get_chain(cc->fat, cc->start_cluster, chain);
    if (nr_clusters > len) {
        /* grow the chain */
        uint32_t new_chain_start_cluster = fat_alloc_chain(cc->fat, nr_clusters - len);
        fat_append_to_chain(cc->fat, cc->start_cluster, new_chain_start_cluster);
    } else {
        /* shrink the chain */
        uint32_t i;
        if (nr_clusters > 0) {
            fat_set_eof(cc->fat, chain[nr_clusters - 1]);
            for (i = nr_clusters; i < len; ++i) {
                fat_set_free(cc->fat, chain[i]);
            }
        } else {
            for (i = 0; i < len; ++i) {
                fat_set_free(cc->fat, chain[i]);
            }

            cc->start_cluster = 0;
        }
    }
}
