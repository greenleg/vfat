#include <math.h>
#include <utility>
#include "../include/ClusterChain.h"

using namespace org::vfat;

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

ClusterChain::ClusterChain(uint32_t startCluster)
    : startCluster(startCluster)
{ }

ClusterChain::ClusterChain() 
    : startCluster(0)
{}

ClusterChain::ClusterChain(const ClusterChain& other) 
  : startCluster(other.startCluster)
{ }

ClusterChain::ClusterChain(ClusterChain&& other)
  : startCluster(std::exchange(other.startCluster, 0))
{ }

ClusterChain& ClusterChain::operator=(const ClusterChain& other)
{
    if (this != &other) {
        startCluster = other.startCluster;
    }

    return *this;
}

ClusterChain& ClusterChain::operator=(ClusterChain&& other)
{
    if (this != &other) {
        startCluster = std::exchange(other.startCluster, 0);
    }

    return *this;
}

uint64_t ClusterChain::GetDeviceOffset(const BootSector& bs, uint32_t cluster, uint32_t clusterOffset) const
{
    uint32_t dataOffset = bs.GetClusterHeapOffset();
    uint32_t clusterSize = bs.GetBytesPerCluster();
    return dataOffset + cluster * clusterSize + clusterOffset;
}

uint32_t ClusterChain::GetLength(const Fat& fat) const
{
    if (this->startCluster == 0) {
        return 0;
    }

    return fat.GetChainLength(this->startCluster);
}

uint64_t ClusterChain::GetSizeInBytes(const Fat& fat) const
{
    uint32_t clusterCount = this->GetLength(fat);
    uint32_t clusterSize = fat.GetBootSector().GetBytesPerCluster();

    return clusterCount * clusterSize;
}

uint32_t ClusterChain::SetSizeInBytes(Fat& fat, uint32_t size)
{
    uint32_t clusterSize = fat.GetBootSector().GetBytesPerCluster();
    uint32_t clusterCount = (size + clusterSize - 1) / clusterSize;
    this->SetLength(fat, clusterCount);

    return clusterSize * clusterCount;
}


void ClusterChain::ReadData(const Device& device, const Fat& fat, uint32_t offset, uint32_t nbytes, uint8_t *buffer) const
{
    if (this->startCluster == 0 && nbytes > 0) {
        // Cannot read from an empty cluster chain
    }

    const BootSector& bootSector = fat.GetBootSector();
    uint32_t clusterSize = bootSector.GetBytesPerCluster();
    uint32_t chainIndex;
    uint32_t bytesLeft;

    uint32_t chain[fat.GetChainLength(this->startCluster)];
    fat.GetChain(this->startCluster, chain);

    chainIndex = (offset / clusterSize);
    bytesLeft = nbytes;

    if (offset % clusterSize != 0) {
        uint32_t cluster_offset = (offset % clusterSize);
        uint32_t size = MIN(clusterSize - cluster_offset, bytesLeft);
        device.Read(buffer, this->GetDeviceOffset(bootSector, chain[chainIndex], cluster_offset), size);
        buffer += size;
        bytesLeft -= size;
        chainIndex++;
    }

    while (bytesLeft > 0) {
        uint32_t size = MIN(clusterSize, bytesLeft);
        device.Read(buffer, this->GetDeviceOffset(bootSector, chain[chainIndex], 0), size);
        buffer += size;
        bytesLeft -= size;
        chainIndex++;
    }
}

void ClusterChain::WriteData(Device& device, Fat& fat, uint32_t offset, uint32_t nbytes, uint8_t *buffer)
{
    if (nbytes == 0) {
        return;
    }

    const BootSector& bootSector = fat.GetBootSector();
    uint32_t clusterSize = bootSector.GetBytesPerCluster();
    uint32_t minSize = offset + nbytes;
    uint32_t chainIndex;
    uint32_t bytesLeft;
    uint32_t clusterOffset;
    uint32_t size;

    if (this->GetSizeInBytes(fat) < minSize) {
        this->SetSizeInBytes(fat, minSize); // growing the chain
    }

    uint32_t chain[fat.GetChainLength(this->startCluster)];
    fat.GetChain(this->startCluster, chain);

    chainIndex = (offset / clusterSize);
    bytesLeft = nbytes;

    if (offset % clusterSize != 0) {
        clusterOffset = (offset % clusterSize);
        size = MIN(clusterSize - clusterOffset, bytesLeft);
        device.Write(buffer, this->GetDeviceOffset(bootSector, chain[chainIndex], clusterOffset), size);
        buffer += size;
        bytesLeft -= size;
        chainIndex++;
    }

    while (bytesLeft > 0) {
        size = MIN(clusterSize, bytesLeft);
        device.Write(buffer, this->GetDeviceOffset(bootSector, chain[chainIndex], 0), size);
        buffer += size;
        bytesLeft -= size;
        chainIndex++;
    }
}

void ClusterChain::SetLength(Fat& fat, uint32_t clusterCount)
{
    if (this->startCluster == 0 && clusterCount == 0) {
        // Nothing to do
        return;
    }

    if (this->startCluster == 0 && clusterCount > 0) {
        this->startCluster = fat.AllocateChain(clusterCount);
        return;
    }

    uint32_t len = fat.GetChainLength(this->startCluster);
    if (clusterCount == len) {
        return;
    }

    uint32_t chain[len];
    fat.GetChain(this->startCluster, chain);
    if (clusterCount > len) {
        // Grow the chain
        uint32_t newChainStartCluster = fat.AllocateChain(clusterCount - len);
        fat.AppendChain(this->startCluster, newChainStartCluster);
    } else {
        // Shrink the chain
        if (clusterCount > 0) {
            fat.SetEof(chain[clusterCount - 1]);
            for (uint32_t i = clusterCount; i < len; ++i) {
                fat.SetFree(chain[i]);
            }
        } else {
            for (uint32_t i = 0; i < len; ++i) {
                fat.SetFree(chain[i]);
            }

            this->startCluster = 0;
        }
    }
}
