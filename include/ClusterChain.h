#ifndef VFAT_CLUSTER_CHAIN_H
#define VFAT_CLUSTER_CHAIN_H

#include <stdint.h>
#include <stdbool.h>

#include "FileDisk.h"
#include "Fat.h"

namespace org::vfat
{
    class ClusterChain
    {
    private:
        uint32_t startCluster;
        Fat *fat;
        uint64_t GetDeviceOffset(uint32_t cluster, uint32_t clusterOffset) const;

    public:
        ClusterChain(Fat *fat, uint32_t startCluster);
        void ReadData(Device *device, uint32_t offset, uint32_t nbytes, uint8_t *buffer) const;
        void WriteData(Device *device, uint32_t offset, uint32_t nbytes, uint8_t *buffer);
        uint32_t GetLength() const;
        uint64_t GetSizeInBytes() const;
        void SetLength(uint32_t clusterCount);
        uint32_t SetSizeInBytes(uint32_t size);
        uint32_t GetStartCluster() { return this->startCluster; }
        Fat * GetFat() { return this->fat; }
    };
}

#endif /* VFAT_CLUSTER_CHAIN_H */
