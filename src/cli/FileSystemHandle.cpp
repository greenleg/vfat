#include "../../include/FileDisk.h"
#include "../../include/cli/FileSystemHandle.h"

using namespace org::vfat::cli;

FileSystemHandle::FileSystemHandle(string deviceName)
{
    this->dev = new FileDisk(deviceName.c_str());
}

FileSystemHandle::~FileSystemHandle()
{
    if (this->dir != nullptr) {
        delete this->dir;
    }

    if (this->fs != nullptr) {
        delete this->fs;
    }

    delete this->dev;
}

void FileSystemHandle::Format(uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorsPerCluster)
{
    this->dev->Create();
    this->fs = new FileSystem(this->dev);
    this->fs->Format(volumeSize, bytesPerSector, sectorsPerCluster);
    this->dir = Directory::GetRoot(this->fs);
}

void FileSystemHandle::Read()
{
    this->dev->Open();
    this->fs = new FileSystem(this->dev);
    this->fs->Read();
    this->dir = Directory::GetRoot(this->fs);
}

void FileSystemHandle::ChangeDirectory(string path)
{
    Directory *newDir = this->dir->GetDirectory(path);
    delete this->dir;
    this->dir = newDir;
}
