#include <iostream>
#include "../../include/FileDisk.h"
#include "../../include/cli/FileSystemHelper.h"

using namespace org::vfat::cli;

FileSystemHelper::FileSystemHelper(const std::string& deviceName)
{
    this->dev = new FileDisk(deviceName.c_str());
}

FileSystemHelper::~FileSystemHelper()
{
    if (this->path != nullptr) {
        delete this->path;
    }

    if (this->fs != nullptr) {
        delete this->fs;
    }

    this->dev->Close();
}

void FileSystemHelper::Format(uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorsPerCluster)
{
    this->dev->Create();
    this->fs = new FileSystem(this->dev);
    this->fs->Format(volumeSize, bytesPerSector, sectorsPerCluster);
    this->path = new Path();
}

void FileSystemHelper::Read()
{
    this->dev->Open();
    this->fs = new FileSystem(this->dev);
    this->fs->Read();
    this->path = new Path();
}

void FileSystemHelper::ChangeDirectory(const std::string& path)
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

Directory* FileSystemHelper::GetCurrentDirectory() const
{
    return new Directory(this->fs, this->path->Clone());
}
