#ifndef VFAT_CLUSTER_CHAIN_H
#define VFAT_CLUSTER_CHAIN_H

#include <stdint.h>
#include <stdbool.h>

#include "FileDisk.h"
#include "fat.h"

struct cch
{
    u32 start_cluster;
    struct fat *fat;
};

void cch_readdata(org::vfat::FileDisk *disk, struct cch *cc, u32 offset, u32 nbytes, uint8_t *dst);
void cch_writedata(org::vfat::FileDisk *disk, struct cch *cc, u32 offset, u32 nbytes, uint8_t *src);
u32  cch_getlen(struct cch *cc);
u64  cch_getsize(struct cch *cc);
bool cch_setlen(/*in*/ struct cch *cc, /*in*/ u32 nr_clusters);
u32  cch_setsize(struct cch *cc, u32 size);
bool cch_create(struct cch *cc, struct fat *fat, u32 len);

#endif /* VFAT_CLUSTER_CHAIN_H */
