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
uint64_t cluster_chain_get_length(struct cluster_chain *cc);
uint64_t cluster_chain_get_length_on_disk(struct cluster_chain *cc);
void cluster_chain_set_length(struct cluster_chain *cc, uint32_t length);
uint32_t cluster_chain_set_size(struct cluster_chain *cc, uint32_t size);
void cluster_chain_create(struct cluster_chain *cc, struct fat *fat, uint32_t length);

#endif /* VFAT_CLUSTER_CHAIN_H */
