#include <math.h>

#include "clusterchain.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

static u64 get_dev_offset(struct vbr *vbr, u32 cluster, u32 cluster_offset)
{
    u32 cluster_size = vbr_get_bytes_per_cluster(vbr);
    return vbr->cluster_heap_offset + cluster * cluster_size + cluster_offset;
}

u32 cch_getlen(struct cch *cc)
{
    if (cc->start_cluster == 0) {
        return 0;
    }

    return fat_get_chain_length(cc->fat, cc->start_cluster);
}

u64 cch_getsize(struct cch *cc)
{
    if (cc->start_cluster == 0) {
        return 0;
    }

    u32 clus_size = vbr_get_bytes_per_cluster(cc->fat->vbr);
    u32 clus_cnt = fat_get_chain_length(cc->fat, cc->start_cluster);

    return clus_cnt * clus_size;
}

u32 cch_setsize(struct cch *cc, u32 size)
{
    u32 clus_size = vbr_get_bytes_per_cluster(cc->fat->vbr);
    u32 clus_cnt = (size + clus_size - 1) / clus_size;
    cch_setlen(cc, clus_cnt);

    return clus_size * clus_cnt;
}

void cch_create(struct cch *cc, struct fat *fat, u32 length)
{
    cc->fat = fat;
    cc->start_cluster = 0;
    cch_setlen(cc, length);
}

void cch_readdata(struct fdisk *disk, struct cch *cc, u32 offset, u32 nbytes, u8 *dst)
{
    if (cc->start_cluster == 0 && nbytes > 0) {
        /* Cannot read from empty cluster chain */
    }

    struct fat *fat = cc->fat;
    struct vbr *vbr = fat->vbr;
    u32 cluster_size = vbr_get_bytes_per_cluster(vbr);
    u32 chain_idx;
    u32 n;

    u32 chain[fat_get_chain_length(fat, cc->start_cluster)];
    fat_get_chain(fat, cc->start_cluster, chain);

    chain_idx = (offset / cluster_size);
    n = nbytes;

    if (offset % cluster_size != 0) {
        u32 cluster_offset = (offset % cluster_size);
        u32 size = MIN(cluster_size - cluster_offset, n);
        fdisk_read(disk, dst, get_dev_offset(vbr, chain[chain_idx], cluster_offset), size);
        dst += size;
        n -= size;
        ++chain_idx;
    }

    while (n > 0) {
        u32 size = MIN(cluster_size, n);
        fdisk_read(disk, dst, get_dev_offset(vbr, chain[chain_idx], 0), size);
        dst += size;
        n -= size;
        ++chain_idx;
    }
}

void cch_writedata(struct fdisk *disk, struct cch *cc, u32 offset, u32 nbytes, u8 *src)
{
    if (nbytes == 0) {
        return;
    }

    struct fat *fat = cc->fat;
    struct vbr *vbr = fat->vbr;
    u32 cluster_size = vbr_get_bytes_per_cluster(vbr);
    u32 min_size = offset + nbytes;
    u32 chain_idx;
    u32 n;
    u32 cluster_offset;
    u32 size;

    if (cch_getsize(cc) < min_size) {
        cch_setsize(cc, min_size); // growing the chain
    }

    u32 chain[fat_get_chain_length(fat, cc->start_cluster)];
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

void cch_setlen(struct cch *cc, u32 nr_clusters)
{
    if (cc->start_cluster == 0 && nr_clusters == 0) {
        /* nothing to do */
        return;
    }

    if (cc->start_cluster == 0 && nr_clusters > 0) {
        cc->start_cluster = fat_alloc_chain(cc->fat, nr_clusters);
        return;
    }

    u32 len = fat_get_chain_length(cc->fat, cc->start_cluster);
    if (nr_clusters == len) {
        return;
    }

    u32 chain[len];
    fat_get_chain(cc->fat, cc->start_cluster, chain);
    if (nr_clusters > len) {
        /* grow the chain */
        u32 new_chain_start_cluster = fat_alloc_chain(cc->fat, nr_clusters - len);
        fat_append_to_chain(cc->fat, cc->start_cluster, new_chain_start_cluster);
    } else {
        /* shrink the chain */
        u32 i;
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
