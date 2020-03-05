#ifndef VFAT_CLUSTER_CHAIN_H
#define VFAT_CLUSTER_CHAIN_H

#include <stdint.h>
#include <stdbool.h>

#include "FileDisk.h"
#include "fat.h"

struct cch
{
    uint32_t start_cluster;
    struct fat *fat;
};

void cch_readdata(org::vfat::FileDisk *disk, struct cch *cc, uint32_t offset, uint32_t nbytes, uint8_t *dst);
void cch_writedata(org::vfat::FileDisk *disk, struct cch *cc, uint32_t offset, uint32_t nbytes, uint8_t *src);
uint32_t  cch_getlen(struct cch *cc);
uint64_t  cch_getsize(struct cch *cc);
bool cch_setlen(/*in*/ struct cch *cc, /*in*/ uint32_t nr_clusters);
uint32_t  cch_setsize(struct cch *cc, uint32_t size);
bool cch_create(struct cch *cc, struct fat *fat, uint32_t len);

#endif /* VFAT_CLUSTER_CHAIN_H */
