#ifndef VFAT_BOOTRECORD_H
#define VFAT_BOOTRECORD_H

#include <stdint.h>
#include "FileDisk.h"

#define BYTES_PER_SECTOR 512
#define SECTORS_PER_CLUSTER 8
#define FAT_ENTRY_SIZE 4

namespace org::vfat
{
    class BootSector
    {
    private:
        /* The total size of the virtual disk in bytes */
        uint64_t deviceSizeInBytes;

        /* The number of bytes per sector. */
        uint16_t bytesPerSector;

        /* The number of sectors per cluster. */
        uint16_t sectorsPerCluster;

        /* The FAT offset in bytes from the beginning of the storage device. */
        uint32_t fatOffset;

        /* The actual FAT size in bytes. */
        uint32_t fatSizeInBytes;

        /* The offset of the cluster heap (data region) in bytes from the beginning of the storage device. */
        uint32_t clusterHeapOffset;

        /* The total number of clusters in the cluster heap. */
        uint32_t clusterCount;

        /* The index of the first cluster of the root directory. */
        uint32_t rootDirFirstCluster;

        template<class T>
        static bool IsPowerOfTwo(T n) { return (n != 0) && ((n & (n - 1)) == 0); }

    public:
        BootSector();

        void Create(uint64_t volumeSize)
        {
            Create(volumeSize, BYTES_PER_SECTOR, SECTORS_PER_CLUSTER);
        }

        void Create(uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorsPerCluster);
        void Read(const Device& device);
        void Write(Device& device) const;

        /**
         * @brief Gets the total size of the device in bytes.
         * @return A capacity of this device in bytes.
         */
        uint64_t GetDeviceSizeInBytes() const { return this->deviceSizeInBytes; }

        uint32_t GetFatSizeInBytes() const { return this->fatSizeInBytes; }

        uint16_t GetBytesPerSector() const { return this->bytesPerSector; }

        uint16_t GetSectorsPerCluster() const { return this->sectorsPerCluster; }

        uint32_t GetBytesPerCluster() const { return this->bytesPerSector * this->sectorsPerCluster; }

        uint32_t GetClusterHeapOffset() const { return this->clusterHeapOffset; }

        uint32_t GetClusterCount() const { return this->clusterCount; }

        uint32_t GetFatOffset() const { return this->fatOffset; }

        uint32_t GetRootDirFirstCluster() const { return this->rootDirFirstCluster; }

        void SetRootDirFirstCluster(uint32_t val) { this->rootDirFirstCluster = val; }
    };
}

#endif /* VFAT_BOOTRECORD_H */
