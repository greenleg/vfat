#ifndef VFAT_FAT_H
#define VFAT_FAT_H

#include <stdint.h>
#include <stdbool.h>
#include <vector>
#include "BootSector.h"

/*
 * There are a few special values that relate to the FAT:
 * 0x00000000 – No significant meaning
 * 0x00000001 – Not a valid cell value
 * 0xFFFFFFF6 – Largest Value
 * 0xFFFFFFF7 – Bad Block
 * 0xFFFFFFF8 – Media Descriptor
 * 0xFFFFFFF9-0xFFFFFFFE – Not Defined
 * 0xFFFFFFFF – End of File (EOF)
 */

#define FAT_MEDIA_DESCRIPTOR     0xFFFFFFF8
#define FAT_EOF                  0xFFFFFFFF
#define FAT_FREE                 0x00000000
#define FAT_FIRST_CLUSTER        2

using namespace org::vfat;

namespace org::vfat
{
    class Fat
    {
    private:
        BootSector *bootSector;
        std::vector<uint32_t> entries;
        uint32_t lastAllocatedCluster;

        uint32_t AllocateCluster();

    public:
        Fat() = delete;
        Fat(const Fat& other) = delete;
        Fat(Fat&& other) = delete;
        Fat& operator=(const Fat& other) = delete;
        Fat& operator=(Fat&& other) = delete;        
    
        Fat(BootSector *bootSector);
        void Create();
        void Read(Device *device);
        void Write(Device *device) const;
        uint32_t AllocateChain(uint32_t length);
        void AppendChain(uint32_t startCluster1, uint32_t startCluster2);
        uint32_t GetChainLength(uint32_t startCluster) const;
        void GetChain(uint32_t startCluster, uint32_t *chain) const;

        void SetEof(uint32_t cluster);
        void SetFree(uint32_t cluster);
        uint32_t GetFreeClusterCount() const;
        uint32_t GetEntry(int i) const;  // only for testing purpose;

        BootSector * GetBootSector() const { return this->bootSector; }
        uint32_t GetLastAllocatedCluster() const { return this->lastAllocatedCluster; }

        ~Fat();
    };
}

#endif /* VFAT_FAT_H */
