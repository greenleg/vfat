#include <math.h>
#include "../include/cch.h"

using namespace org::vfat;

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

ClusterChain::ClusterChain(Fat *fat, uint32_t startCluster)
{
    this->fat = fat;
    this->startCluster = startCluster;
}

//void ClusterChain::Create(uint32_t length)
//{
//    this->startCluster = 0;
//    this->SetLength(length);
//}

uint64_t ClusterChain::GetDeviceOffset(uint32_t cluster, uint32_t clusterOffset) const
{
    BootSector *bs = this->fat->GetBootSector();
    uint32_t dataOffset = bs->GetClusterHeapOffset();
    uint32_t clusterSize = bs->GetBytesPerCluster();
    return dataOffset + cluster * clusterSize + clusterOffset;
}

uint32_t ClusterChain::GetLength() const
{
    if (this->startCluster == 0) {
        return 0;
    }

    return this->fat->GetChainLength(this->startCluster);
}

uint64_t ClusterChain::GetSizeInBytes() const
{
    uint32_t clusterCount = this->GetLength();
    uint32_t clusterSize = this->fat->GetBootSector()->GetBytesPerCluster();

    return clusterCount * clusterSize;
}

uint32_t ClusterChain::SetSizeInBytes(uint32_t size)
{
    uint32_t clusterSize = this->fat->GetBootSector()->GetBytesPerCluster();
    uint32_t clusterCount = (size + clusterSize - 1) / clusterSize;
    this->SetLength(clusterCount);

    return clusterSize * clusterCount;
}


void ClusterChain::ReadData(FileDisk *device, uint32_t offset, uint32_t nbytes, uint8_t *buffer) const
{
    if (this->startCluster == 0 && nbytes > 0) {
        // Cannot read from an empty cluster chain
    }

    BootSector* bootSector = this->fat->GetBootSector();
    uint32_t clusterSize = bootSector->GetBytesPerCluster();
    uint32_t chainIndex;
    uint32_t bytesLeft;

    uint32_t chain[fat->GetChainLength(this->startCluster)];
    this->fat->GetChain(this->startCluster, chain);

    chainIndex = (offset / clusterSize);
    bytesLeft = nbytes;

    if (offset % clusterSize != 0) {
        uint32_t cluster_offset = (offset % clusterSize);
        uint32_t size = MIN(clusterSize - cluster_offset, bytesLeft);
        device->Read(buffer, this->GetDeviceOffset(chain[chainIndex], cluster_offset), size);
        buffer += size;
        bytesLeft -= size;
        chainIndex++;
    }

    while (bytesLeft > 0) {
        uint32_t size = MIN(clusterSize, bytesLeft);
        device->Read(buffer, this->GetDeviceOffset(chain[chainIndex], 0), size);
        buffer += size;
        bytesLeft -= size;
        chainIndex++;
    }
}

void ClusterChain::WriteData(FileDisk *device, uint32_t offset, uint32_t nbytes, uint8_t *buffer)
{
    if (nbytes == 0) {
        return;
    }

    BootSector *bootSector = this->fat->GetBootSector();
    uint32_t clusterSize = bootSector->GetBytesPerCluster();
    uint32_t minSize = offset + nbytes;
    uint32_t chainIndex;
    uint32_t bytesLeft;
    uint32_t clusterOffset;
    uint32_t size;

    if (this->GetSizeInBytes() < minSize) {
        this->SetSizeInBytes(minSize); // growing the chain
    }

    uint32_t chain[fat->GetChainLength(this->startCluster)];
    fat->GetChain(this->startCluster, chain);

    chainIndex = (offset / clusterSize);
    bytesLeft = nbytes;

    if (offset % clusterSize != 0) {
        clusterOffset = (offset % clusterSize);
        size = MIN(clusterSize - clusterOffset, bytesLeft);
        device->Write(buffer, this->GetDeviceOffset(chain[chainIndex], clusterOffset), size);
        buffer += size;
        bytesLeft -= size;
        chainIndex++;
    }

    while (bytesLeft > 0) {
        size = MIN(clusterSize, bytesLeft);
        device->Write(buffer, this->GetDeviceOffset(chain[chainIndex], 0), size);
        buffer += size;
        bytesLeft -= size;
        chainIndex++;
    }
}

void ClusterChain::SetLength(uint32_t clusterCount)
{
    if (this->startCluster == 0 && clusterCount == 0) {
        // Nothing to do
        return;
    }

    if (this->startCluster == 0 && clusterCount > 0) {
        this->startCluster = this->fat->AllocateChain(clusterCount);
        return;
    }

    uint32_t len = this->fat->GetChainLength(this->startCluster);
    if (clusterCount == len) {
        return;
    }

    uint32_t chain[len];
    this->fat->GetChain(this->startCluster, chain);
    if (clusterCount > len) {
        // Grow the chain
        uint32_t newChainStartCluster = this->fat->AllocateChain(clusterCount - len);
        this->fat->AppendChain(this->startCluster, newChainStartCluster);
    } else {
        // Shrink the chain
        if (clusterCount > 0) {
            this->fat->SetEof(chain[clusterCount - 1]);
            for (uint32_t i = clusterCount; i < len; ++i) {
                this->fat->SetFree(chain[i]);
            }
        } else {
            for (uint32_t i = 0; i < len; ++i) {
                this->fat->SetFree(chain[i]);
            }

            this->startCluster = 0;
        }
    }
}
