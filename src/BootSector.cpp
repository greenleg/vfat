#include <stdint.h>
#include <sys/types.h>
#include <math.h>
#include <sstream>
#include <stdexcept>

#include "../include/BinaryReader.h"
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

void BootSector::Create(uint64_t deviceSizeInBytes, uint16_t bytesPerSector, uint16_t sectorsPerCluster)
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

    this->deviceSizeInBytes = deviceSizeInBytes;
    this->bytesPerSector = bytesPerSector;
    this->sectorsPerCluster = sectorsPerCluster;
    this->fatOffset = this->bytesPerSector;

    uint32_t bytesPerCluster = this->bytesPerSector * this->sectorsPerCluster;

    // Adjust the volume size to be a multiple of the sector size.
    uint64_t volumeSizeInSectors = this->deviceSizeInBytes / this->bytesPerSector;
    int adjustedVolumeSizeInBytes = volumeSizeInSectors * this->bytesPerSector;

    this->clusterCount = (adjustedVolumeSizeInBytes - this->fatOffset) / (FAT_ENTRY_SIZE + bytesPerCluster);
    this->fatSizeInBytes = this->clusterCount * FAT_ENTRY_SIZE;

    // Adjust the FAT size to be a multiple of the sector size.
    uint32_t adjustedFatSizeInSectors = (this->fatSizeInBytes + this->bytesPerSector - 1) / this->bytesPerSector;
    uint32_t adjustedFatSizeInBytes = adjustedFatSizeInSectors * this->bytesPerSector;
    this->clusterHeapOffset = this->fatOffset + adjustedFatSizeInBytes;

    this->rootDirFirstCluster = 2;
}

void BootSector::Read(const Device& device)
{
    uint8_t buffer[BS_HEADER_SIZE];
    device.Read(buffer, 0, BS_HEADER_SIZE);

    this->deviceSizeInBytes = BinaryReader::ReadUInt64(buffer, BS_VOLUMELENGTH_OFFSET);
    this->bytesPerSector = BinaryReader::ReadUInt16(buffer, BS_BYTESPERSECTOR_OFFSET);
    this->sectorsPerCluster = BinaryReader::ReadUInt16(buffer, BS_SECTORSPERCLUSTER_OFFSET);
    this->fatOffset = BinaryReader::ReadUInt32(buffer, BS_FATOFFSET_OFFSET);
    this->fatSizeInBytes = BinaryReader::ReadUInt32(buffer, BS_FATLENGTH_OFFSET);
    this->clusterCount = BinaryReader::ReadUInt32(buffer, BS_CLUSTERCOUNT_OFFSET);
    this->clusterHeapOffset = BinaryReader::ReadUInt32(buffer, BS_CLUSTERHEAPOFFSET_OFFSET);
    this->rootDirFirstCluster = BinaryReader::ReadUInt32(buffer, BS_ROOTDIRFIRSTCLUSTER_OFFSET);
}


void BootSector::Write(Device& device) const
{
    uint8_t buffer[BS_HEADER_SIZE];

    BinaryReader::WriteUInt64(buffer, BS_VOLUMELENGTH_OFFSET, this->deviceSizeInBytes);
    BinaryReader::WriteUInt16(buffer, BS_BYTESPERSECTOR_OFFSET, this->bytesPerSector);
    BinaryReader::WriteUInt16(buffer, BS_SECTORSPERCLUSTER_OFFSET, this->sectorsPerCluster);
    BinaryReader::WriteUInt32(buffer, BS_FATOFFSET_OFFSET, this->fatOffset);
    BinaryReader::WriteUInt32(buffer, BS_FATLENGTH_OFFSET, this->fatSizeInBytes);
    BinaryReader::WriteUInt32(buffer, BS_CLUSTERCOUNT_OFFSET, this->clusterCount);
    BinaryReader::WriteUInt32(buffer, BS_CLUSTERHEAPOFFSET_OFFSET, this->clusterHeapOffset);
    BinaryReader::WriteUInt32(buffer, BS_ROOTDIRFIRSTCLUSTER_OFFSET, this->rootDirFirstCluster);

    device.Write(buffer, 0, BS_HEADER_SIZE);
}
