#include "../../include/FileDisk.h"
#include "../../include/cli/FileSystemHandle.h"

using namespace org::vfat::cli;

FileSystemHandle::FileSystemHandle(string deviceName)
{
    this->dev = new FileDisk(deviceName.c_str());
}

FileSystemHandle::~FileSystemHandle()
{
    if (this->path != nullptr) {
        delete this->path;
    }

    if (this->fs != nullptr) {
        delete this->fs;
    }

    this->dev->Close();
    delete this->dev;
}

void FileSystemHandle::Format(uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorsPerCluster)
{
    this->dev->Create();
    this->fs = new FileSystem(this->dev);
    this->fs->Format(volumeSize, bytesPerSector, sectorsPerCluster);
    this->dir = Directory::GetRoot(this->fs);
    this->path = new Path();
}

void FileSystemHandle::Read()
{
    this->dev->Open();
    this->fs = new FileSystem(this->dev);
    this->fs->Read();
    this->dir = Directory::GetRoot(this->fs);
    this->path = new Path();
}

void FileSystemHandle::ChangeDirectory(string path)
{
//    Path currentPath;
//    currentPath.Combine(this->path->ToString());
//    currentPath.Combine(path);

    //Directory *rootDir = Directory::GetRoot(this->fs);
    Directory *newDir = this->dir->GetDirectory(path);
    delete this->dir;
    this->dir = newDir;
}
