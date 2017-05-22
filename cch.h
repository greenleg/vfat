#ifndef VFAT_CLUSTER_CHAIN_H
#define VFAT_CLUSTER_CHAIN_H

#include <stdint.h>

#include "fdisk.h"
#include "fat.h"

struct cch
{
    u32 start_cluster;
    struct fat *fat;
};

void cch_readdata(struct fdisk *disk, struct cch *cc, u32 offset, u32 nbytes, u8 *dst);
void cch_writedata(struct fdisk *disk, struct cch *cc, u32 offset, u32 nbytes, u8 *src);
u32  cch_getlen(struct cch *cc);
u64  cch_getsize(struct cch *cc);
void cch_setlen(struct cch *cc, uint32_t len);
u32  cch_setsize(struct cch *cc, u32 size);
void cch_create(struct cch *cc, struct fat *fat, u32 len);

#endif /* VFAT_CLUSTER_CHAIN_H */
