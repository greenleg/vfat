#ifndef VFAT_FILESYSTEM_H
#define VFAT_FILESYSTEM_H

#include <string>
#include <vector>

#include "../FileDisk.h"
#include "../Fat.h"
#include "../ClusterChainDirectory.h"

using namespace org::vfat;

namespace org::vfat::api
{
    class FileSystem
    {
    private:
        FileDisk *device;
        BootSector *bootSector;
        Fat *fat;

    public:
        FileSystem(FileDisk *device);
        ~FileSystem();
        void Format(uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorsPerCluster);
        void Read();
        void Write();

        FileDisk* GetDevice() const { return this->device; }
        BootSector* GetBootSector() const { return this->bootSector; }
        Fat* GetFat() const { return this->fat; }
        ClusterChainDirectory* GetRootDirectory() const;// { return this->root; }
    };
}

#endif /* VFAT_FILESYSTEM_H */
