#include <math.h>
#include "../include/cch.h"

using namespace org::vfat;

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

static uint64_t getdevofs(BootSector *bootSector, uint32_t cluster, uint32_t cluster_offset)
{
    uint32_t dataOffset = bootSector->GetClusterHeapOffset();
    uint32_t clusterSize = bootSector->GetBytesPerCluster();
    return dataOffset + cluster * clusterSize + cluster_offset;
}

uint32_t cch_getlen(struct cch *cc)
{
    if (cc->start_cluster == 0) {
        return 0;
    }

    return fat_getchainlen(cc->fat, cc->start_cluster);
}

uint64_t cch_getsize(struct cch *cc)
{
    uint32_t clus_cnt = cch_getlen(cc);
    uint32_t clusterSize = cc->fat->bootSector->GetBytesPerCluster();

    return clus_cnt * clusterSize;
}

uint32_t cch_setsize(struct cch *cc, uint32_t size)
{
    uint32_t clusterSize = cc->fat->bootSector->GetBytesPerCluster();
    uint32_t clus_cnt = (size + clusterSize - 1) / clusterSize;
    cch_setlen(cc, clus_cnt);

    return clusterSize * clus_cnt;
}

bool cch_create(/*in*/ struct cch *cc, /*in*/ struct fat *fat, /*in*/ uint32_t length)
{
    cc->fat = fat;
    cc->start_cluster = 0;
    return cch_setlen(cc, length);
}

void cch_readdata(org::vfat::FileDisk *device, struct cch *cc, uint32_t offset, uint32_t nbytes, uint8_t *buffer)
{
    if (cc->start_cluster == 0 && nbytes > 0) {
        /* Cannot read from an empty cluster chain */
    }

    struct fat *fat = cc->fat;
    BootSector* bootSector = fat->bootSector;
    uint32_t clusterSize = bootSector->GetBytesPerCluster();
    uint32_t chain_idx;
    uint32_t n;

    uint32_t chain[fat_getchainlen(fat, cc->start_cluster)];
    fat_getchain(fat, cc->start_cluster, chain);

    chain_idx = (offset / clusterSize);
    n = nbytes;

    if (offset % clusterSize != 0) {
        uint32_t cluster_offset = (offset % clusterSize);
        uint32_t size = MIN(clusterSize - cluster_offset, n);
        device->Read(buffer, getdevofs(bootSector, chain[chain_idx], cluster_offset), size);
        buffer += size;
        n -= size;
        ++chain_idx;
    }

    while (n > 0) {
        uint32_t size = MIN(clusterSize, n);
        device->Read(buffer, getdevofs(bootSector, chain[chain_idx], 0), size);
        buffer += size;
        n -= size;
        ++chain_idx;
    }
}

void cch_writedata(org::vfat::FileDisk *device, struct cch *cc, uint32_t offset, uint32_t nbytes, uint8_t *buffer)
{
    if (nbytes == 0) {
        return;
    }

    struct fat *fat = cc->fat;
    BootSector *bootSector = fat->bootSector;
    uint32_t clusterSize = bootSector->GetBytesPerCluster();
    uint32_t min_size = offset + nbytes;
    uint32_t chain_idx;
    uint32_t n;
    uint32_t cluster_offset;
    uint32_t size;

    if (cch_getsize(cc) < min_size) {
        cch_setsize(cc, min_size); // growing the chain
    }

    uint32_t chain[fat_getchainlen(fat, cc->start_cluster)];
    fat_getchain(fat, cc->start_cluster, chain);

    chain_idx = (offset / clusterSize);
    n = nbytes;

    if (offset % clusterSize != 0) {
        cluster_offset = (offset % clusterSize);
        size = MIN(clusterSize - cluster_offset, n);
        device->Write(buffer, getdevofs(bootSector, chain[chain_idx], cluster_offset), size);
        buffer += size;
        n -= size;
        ++chain_idx;
    }

    while (n > 0) {
        size = MIN(clusterSize, n);
        device->Write(buffer, getdevofs(bootSector, chain[chain_idx], 0), size);
        buffer += size;
        n -= size;
        ++chain_idx;
    }
}

bool cch_setlen(/*in*/ struct cch *cc, /*in*/ uint32_t nr_clusters)
{
    if (cc->start_cluster == 0 && nr_clusters == 0) {
        /* nothing to do */
        return true;
    }

    if (cc->start_cluster == 0 && nr_clusters > 0) {
        return fat_alloc_chain(cc->fat, nr_clusters, &(cc->start_cluster));
    }

    uint32_t len = fat_getchainlen(cc->fat, cc->start_cluster);
    if (nr_clusters == len) {
        return true;
    }

    uint32_t chain[len];
    fat_getchain(cc->fat, cc->start_cluster, chain);
    if (nr_clusters > len) {
        /* grow the chain */
        uint32_t new_chain_start_cluster;
        if (!fat_alloc_chain(cc->fat, nr_clusters - len, &new_chain_start_cluster)) {
            return false;
        }

        fat_append_chain(cc->fat, cc->start_cluster, new_chain_start_cluster);
    } else {
        /* shrink the chain */
        uint32_t i;
        if (nr_clusters > 0) {
            fat_seteof(cc->fat, chain[nr_clusters - 1]);
            for (i = nr_clusters; i < len; ++i) {
                fat_setfree(cc->fat, chain[i]);
            }
        } else {
            for (i = 0; i < len; ++i) {
                fat_setfree(cc->fat, chain[i]);
            }

            cc->start_cluster = 0;
        }
    }

    return true;
}
