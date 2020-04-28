#include <iostream>
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
    //this->dir = Directory::GetRoot(this->fs);
    this->path = new Path();
}

void FileSystemHandle::Read()
{
    this->dev->Open();
    this->fs = new FileSystem(this->dev);
    this->fs->Read();
    //this->dir = Directory::GetRoot(this->fs);
    this->path = new Path();
}

void FileSystemHandle::ChangeDirectory(string path)
{
    Path *newPath = this->path->Clone();
    newPath->Combine(path, false);

    // Check availability of the new path;
    Directory *newDir = new Directory(this->fs, newPath);
    delete newDir;

    Path *newNormalizedPath = this->path->Clone();
    newNormalizedPath->Combine(path, true);

    delete this->path;
    this->path = newNormalizedPath;
}

Directory* FileSystemHandle::GetCurrentDirectory() const
{
    return new Directory(this->fs, this->path->Clone());
}
