#ifndef VFAT_CLUSTER_CHAIN_H
#define VFAT_CLUSTER_CHAIN_H

#include <stdint.h>

#include "fdisk.h"
#include "fat.h"

struct cluster_chain
{
    uint32_t start_cluster;
    struct fat *fat;
};

void cluster_chain_read_data(struct fdisk *disk, struct cluster_chain *cc, uint32_t offset, uint32_t nbytes, uint8_t *dst);
void cluster_chain_write_data(struct fdisk *disk, struct cluster_chain *cc, uint32_t offset, uint32_t nbytes, uint8_t *src);
void cluster_chain_set_length(struct cluster_chain *cc, uint32_t nr_clusters);


#endif /* VFAT_CLUSTER_CHAIN_H */
