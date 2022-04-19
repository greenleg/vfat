#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdexcept>
#include <iostream>
#include "../include/Fat.h"

using namespace org::vfat;

Fat::Fat(BootSector& bootSector)
  : bootSector(bootSector), entries(bootSector.GetClusterCount()), lastAllocatedCluster(0)
{
}

Fat::~Fat()
{
}

void Fat::Create()
{
    this->lastAllocatedCluster = FAT_FIRST_CLUSTER - 1;
    std::fill(this->entries.begin() + 2, this->entries.end(), FAT_FREE);
    
    this->entries[0] = FAT_MEDIA_DESCRIPTOR;
    this->entries[1] = FAT_EOF;
}

void Fat::Read(const Device& device)
{
    this->lastAllocatedCluster = FAT_FIRST_CLUSTER - 1;
    uint32_t fatOffset = this->bootSector.GetFatOffset();
    uint32_t clusterCount = this->bootSector.GetClusterCount();
    device.Read((uint8_t *)this->entries.data(), fatOffset, sizeof(uint32_t) * clusterCount);
}

void Fat::Write(Device& device) const
{
    uint32_t fatOffset = this->bootSector.GetFatOffset();
    uint32_t clusterCount = this->bootSector.GetClusterCount();
    device.Write((uint8_t *)this->entries.data(), fatOffset, sizeof(uint32_t) * clusterCount);
}

uint32_t Fat::AllocateCluster()
{
    for (uint32_t i = this->lastAllocatedCluster + 1; i < this->bootSector.GetClusterCount(); ++i) {
        if (this->entries[i] == FAT_FREE) {
            this->entries[i] = FAT_EOF;
            this->lastAllocatedCluster = i;
            return i;
        }
    }

    for (uint32_t i = FAT_FIRST_CLUSTER; i < this->lastAllocatedCluster; ++i) {
        if (this->entries[i] == FAT_FREE) {
            this->entries[i] = FAT_EOF;
            this->lastAllocatedCluster = i;
            return i;
        }
    }

    throw std::runtime_error("Couldn't find a free cluster. FAT is full.");
}

uint32_t Fat::AllocateChain(uint32_t length)
{
    uint32_t startCluster = this->AllocateCluster();
    uint32_t cluster = startCluster;
    for (uint32_t i = 1; i < length; ++i) {
        uint32_t newCluster = this->AllocateCluster();
        this->entries[cluster] = newCluster;
        cluster = newCluster;
    }

    return startCluster;
}

void Fat::AppendChain(uint32_t startCluster1, uint32_t startCluster2)
{
    uint32_t cluster = startCluster1;

    // Look for the last cluster of the first chain.
    while (this->entries[cluster] != FAT_EOF) {
        cluster = this->entries[cluster];
    }

    // Attach a second cluster chain.
    this->entries[cluster] = startCluster2;
}

uint32_t Fat::GetChainLength(uint32_t startCluster) const
{
    uint32_t cluster = startCluster;
    uint32_t len = 1;
    while (this->entries[cluster] != FAT_EOF) {
        cluster = this->entries[cluster];
        len++;
    }

    return len;
}

void Fat::GetChain(uint32_t startCluster, uint32_t *chain) const
{
    uint32_t cluster = startCluster;
    int i = 0;
    chain[0] = startCluster;
    while (this->entries[cluster] != FAT_EOF) {
        cluster = this->entries[cluster];
        chain[++i] = cluster;
    }
}

void Fat::SetEof(uint32_t cluster)
{
    this->entries[cluster] = FAT_EOF;
}

void Fat::SetFree(uint32_t cluster)
{
    this->entries[cluster] = FAT_FREE;
}

uint32_t Fat::GetEntry(int i) const
{
    return this->entries[i];
}

uint32_t Fat::GetFreeClusterCount() const
{
    uint32_t count = 0;
    for (uint32_t i = FAT_FIRST_CLUSTER; i < this->bootSector.GetClusterCount(); ++i) {
        if (this->entries[i] == FAT_FREE) {
            count++;
        }
    }

    return count;
}
