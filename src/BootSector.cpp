#include <stdint.h>
#include <sys/types.h>
#include <math.h>
#include <sstream>
#include <stdexcept>

#include "../include/binaryreader.h"
#include "../include/BootSector.h"

#define BS_HEADER_SIZE 32
#define BS_VOLUMELENGTH_OFFSET 0
#define BS_BYTESPERSECTOR_OFFSET 8
#define BS_SECTORSPERCLUSTER_OFFSET 10
#define BS_FATOFFSET_OFFSET 12
#define BS_FATLENGTH_OFFSET 16
#define BS_CLUSTERCOUNT_OFFSET 20
#define BS_CLUSTERHEAPOFFSET_OFFSET 24
#define BS_ROOTDIRFIRSTCLUSTER_OFFSET 28

using namespace org::vfat;

BootSector::BootSector() {}

void BootSector::Create(uint64_t volumeSizeInBytes, uint16_t bytesPerSector, uint16_t sectorsPerCluster)
{
    if (!IsPowerOfTwo(bytesPerSector)) {
        std::ostringstream msgStream;
        msgStream << "The number of bytes per sector " << bytesPerSector << " should be a power of two.";
        throw std::runtime_error(msgStream.str());
    }

    if (!IsPowerOfTwo(sectorsPerCluster)) {
        std::ostringstream msgStream;
        msgStream << "The number of sectors per cluster " << sectorsPerCluster << " should be a power of two.";
        throw std::runtime_error(msgStream.str());
    }

//    if (bytesPerSector % FAT_ENTRY_SIZE != 0) {
//        std::ostringstream msgStream;
//        msgStream << "The number of bytes per sector " << bytesPerSector << " should be a multiple of FAT entry size " << FAT_ENTRY_SIZE << " bytes.";
//        throw std::runtime_error(msgStream.str());
//    }

    this->volumeSizeInBytes = volumeSizeInBytes;
    this->bytesPerSector = bytesPerSector;
    this->sectorsPerCluster = sectorsPerCluster;
    this->fatOffset = this->bytesPerSector;

    uint32_t bytesPerCluster = this->bytesPerSector * this->sectorsPerCluster;

    // Adjust the volume size to be a multiple of the sector size.
    uint64_t volumeSizeInSectors = this->volumeSizeInBytes / this->bytesPerSector;
    int adjustedVolumeSizeInBytes = volumeSizeInSectors * this->bytesPerSector;

    this->clusterCount = (adjustedVolumeSizeInBytes - this->fatOffset) / (FAT_ENTRY_SIZE + bytesPerCluster);
    this->fatSizeInBytes = this->clusterCount * FAT_ENTRY_SIZE;

    // Adjust the FAT size to be a multiple of the sector size.
    uint32_t adjustedFatSizeInSectors = (this->fatSizeInBytes + this->bytesPerSector - 1) / this->bytesPerSector;
    uint32_t adjustedFatSizeInBytes = adjustedFatSizeInSectors * this->bytesPerSector;
    this->clusterHeapOffset = this->fatOffset + adjustedFatSizeInBytes;

    this->rootDirFirstCluster = 2;
}

void BootSector::Read(FileDisk *device)
{
    uint8_t buffer[BS_HEADER_SIZE];
    device->Read(buffer, 0, BS_HEADER_SIZE);

    this->volumeSizeInBytes = read_u64(buffer, BS_VOLUMELENGTH_OFFSET);
    this->bytesPerSector = read_u16(buffer, BS_BYTESPERSECTOR_OFFSET);
    this->sectorsPerCluster = read_u16(buffer, BS_SECTORSPERCLUSTER_OFFSET);
    this->fatOffset = read_u32(buffer, BS_FATOFFSET_OFFSET);
    this->fatSizeInBytes = read_u32(buffer, BS_FATLENGTH_OFFSET);
    this->clusterCount = read_u32(buffer, BS_CLUSTERCOUNT_OFFSET);
    this->clusterHeapOffset = read_u32(buffer, BS_CLUSTERHEAPOFFSET_OFFSET);
    this->rootDirFirstCluster = read_u32(buffer, BS_ROOTDIRFIRSTCLUSTER_OFFSET);
}


void BootSector::Write(FileDisk *device) const
{
    uint8_t buffer[BS_HEADER_SIZE];

    write_u64(buffer, BS_VOLUMELENGTH_OFFSET, this->volumeSizeInBytes);
    write_u16(buffer, BS_BYTESPERSECTOR_OFFSET, this->bytesPerSector);
    write_u16(buffer, BS_SECTORSPERCLUSTER_OFFSET, this->sectorsPerCluster);
    write_u32(buffer, BS_FATOFFSET_OFFSET, this->fatOffset);
    write_u32(buffer, BS_FATLENGTH_OFFSET, this->fatSizeInBytes);
    write_u32(buffer, BS_CLUSTERCOUNT_OFFSET, this->clusterCount);
    write_u32(buffer, BS_CLUSTERHEAPOFFSET_OFFSET, this->clusterHeapOffset);
    write_u32(buffer, BS_ROOTDIRFIRSTCLUSTER_OFFSET, this->rootDirFirstCluster);

    device->Write(buffer, 0, BS_HEADER_SIZE);
}
