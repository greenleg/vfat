#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include "../../include/api/FileSystem.h"
#include "../../include/api/Directory.h"
#include "../../include/Common.h"

using namespace org::vfat;
using namespace org::vfat::api;

FileSystem::FileSystem(Device &device)
  : device(device)
{
}

void FileSystem::Format(uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorsPerCluster)
{
//    this->bootSector = new BootSector();
    this->bootSector.Create(volumeSize, bytesPerSector, sectorsPerCluster);

    this->fat = new Fat(this->bootSector);
    this->fat->Create();

    this->bootSector.Write(this->device);
    this->fat->Write(this->device);

    ClusterChainDirectory root;
    root.CreateRoot(this->fat);
    root.Write(this->device);
}

void FileSystem::Read()
{
//    this->bootSector = new BootSector();
    this->bootSector.Read(this->device);

    this->fat = new Fat(this->bootSector);
    this->fat->Read(this->device);
}

void FileSystem::Write()
{
    this->fat->Write(this->device);
    this->bootSector.Write(this->device);
}

FileSystem::~FileSystem()
{
    delete this->fat;
//    delete this->bootSector;
}

ClusterChainDirectory FileSystem::GetRootDirectory() const
{
    ClusterChainDirectory root;
    root.ReadRoot(device, this->fat);
    return root;
}
