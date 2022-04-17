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
        Device& device;
        BootSector bootSector;
        std::unique_ptr<Fat> fat;

    public:
        FileSystem(Device& device);
        ~FileSystem();
        void Format(uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorsPerCluster);
        void Read();
        void Write();

        Device& GetDevice() const { return this->device; }
        const BootSector& GetBootSector() const { return this->bootSector; }
        Fat& GetFat() { return *(this->fat); }
        ClusterChainDirectory GetRootDirectory() const;
    };
}

#endif /* VFAT_FILESYSTEM_H */
