#ifndef VFAT_BOOTRECORD_H
#define VFAT_BOOTRECORD_H

#include <stdint.h>
//#include "common.h"
#include "FileDisk.h"

#define BYTES_PER_SECTOR 512
#define SECTORS_PER_CLUSTER 8
#define FAT_ENTRY_SIZE 4

//struct vbr
//{
//    /* Size of total volume in sectors */
//    u64 volume_length;

//    /* Sector address of 1st FAT */
//    u32 fat_offset;

//    /* Size of FAT in Sectors */
//    u32 fat_length;

//    /* Sector address of the Data Region */
//    u32 cluster_heap_offset;

//    /* Number of clusters in the Cluster Heap */
//    u32 cluster_count;

//    /* Cluster address of the Root Directory */
//    u32 rootdir_first_cluster;

//    /* This is a power of 2. Range: min of 29 = 512 byte cluster size, and a max of 212 = 4096. */
//    uint8_t bytes_per_sector_pow2;

//    /*
//     * This is a power of 2. Range: Min of 21=512. The maximum Cluster size is 32 MiB,
//     * so the Values in Bytes per Sector + Sectors Per Cluster cannot exceed 25.
//     */
//    uint8_t sectors_per_cluster_pow2;
//};

namespace org::vfat
{
    class BootSector
    {
    private:
        /* The total size of volume in bytes */
        uint64_t volumeSizeInBytes;

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
        static bool IsPowerOfTwo(T n)
        {
            return (n != 0) && ((n & (n - 1)) == 0);
        }

    public:
        BootSector();

        void Create(uint64_t volumeSize)
        {
            Create(volumeSize, BYTES_PER_SECTOR, SECTORS_PER_CLUSTER);
        }

        void Create(uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorsPerCluster);
        void Read(FileDisk *device);
        void Write(FileDisk *device) const;

        uint64_t GetVolumeSizeInBytes() const
        {
            return this->volumeSizeInBytes;
        }

        uint32_t GetFatSizeInBytes() const
        {
            return this->fatSizeInBytes;
        }

        uint16_t GetBytesPerSector() const
        {
            return this->bytesPerSector;
        }

        uint16_t GetSectorsPerCluster() const {
            return this->sectorsPerCluster;
        }

        uint32_t GetBytesPerCluster() const
        {
            return this->bytesPerSector * this->sectorsPerCluster;
        }

        uint32_t GetClusterHeapOffset() const
        {
            return this->clusterHeapOffset;
        }

        uint32_t GetClusterCount() const
        {
            return this->clusterCount;
        }

        uint32_t GetFatOffset() const
        {
            return this->fatOffset;
        }

        uint32_t GetRootDirFirstCluster()
        {
            return this->rootDirFirstCluster;
        }

        void SetRootDirFirstCluster(uint32_t val)
        {
            this->rootDirFirstCluster = val;
        }
    };
}

//void vbr_read(org::vfat::FileDisk *device, struct vbr *vbr);
//void vbr_write(struct vbr *vbr, org::vfat::FileDisk *device);
//void vbr_create(struct vbr *vbr, u64 volume_size, u16 bytes_per_sector, u16 sector_per_cluster);

//u16  vbr_get_bytes_per_sector(struct vbr *vbr);
//void vbr_set_bytes_per_sector(struct vbr *vbr, u16 val);
//u16  vbr_get_sectors_per_cluster(struct vbr *vbr);
//void vbr_set_sectors_per_cluster(struct vbr *vbr, u16 val);
//u32  vbr_get_bytes_per_cluster(struct vbr * vbr);

#endif /* VFAT_BOOTRECORD_H */
