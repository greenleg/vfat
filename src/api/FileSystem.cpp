#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include "../../include/api/FileSystem.h"
#include "../../include/api/Directory.h"
#include "../../include/Common.h"

using namespace org::vfat;
using namespace org::vfat::api;

FileSystem::FileSystem(Device& device)
  : device(device), bootSector(), fat(nullptr)
{
}

void FileSystem::Format(uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorsPerCluster)
{
    this->bootSector.Create(volumeSize, bytesPerSector, sectorsPerCluster);

    this->fat = std::make_unique<Fat>(this->bootSector);
    this->fat->Create();

    ClusterChainDirectory root;
    root.CreateRoot(*(this->fat));
    root.Write(this->device, *(this->fat));
    
    this->fat->Write(this->device);
    this->bootSector.Write(this->device);
}

void FileSystem::Read()
{
    this->bootSector.Read(this->device);
    this->fat = std::make_unique<Fat>(this->bootSector);
    this->fat->Read(this->device);
}

void FileSystem::Write()
{
    this->fat->Write(this->device);
    this->bootSector.Write(this->device);
}

FileSystem::~FileSystem()
{
}

ClusterChainDirectory FileSystem::GetRootDirectory() const
{
    ClusterChainDirectory root;
    root.ReadRoot(device, *(this->fat));
    return root;
}
