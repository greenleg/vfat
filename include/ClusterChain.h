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
//        Fat *fat;
        
        uint64_t GetDeviceOffset(const BootSector& bs, uint32_t cluster, uint32_t clusterOffset) const;

    public:
        ClusterChain();
        ClusterChain(const ClusterChain& other);
        ClusterChain(ClusterChain&& other);
        ClusterChain& operator=(const ClusterChain& other);
        ClusterChain& operator=(ClusterChain&& other);
        ClusterChain(uint32_t startCluster);
        void ReadData(const Device& device, const Fat& fat, uint32_t offset, uint32_t nbytes, uint8_t *buffer) const;
        void WriteData(Device& device, Fat& fat, uint32_t offset, uint32_t nbytes, uint8_t *buffer);
        uint32_t GetLength(const Fat& fat) const;
        uint64_t GetSizeInBytes(const Fat& fat) const;
        void SetLength(Fat& fat, uint32_t clusterCount);
        uint32_t SetSizeInBytes(Fat& fat, uint32_t size);
        uint32_t GetStartCluster() const { return this->startCluster; }
//        Fat * GetFat() { return this->fat; }
    };
}

#endif /* VFAT_CLUSTER_CHAIN_H */
