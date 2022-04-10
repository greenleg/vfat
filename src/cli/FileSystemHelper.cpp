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
    if (this->fs != nullptr) {
        delete this->fs;
    }

    this->dev->Close();
    delete this->dev;
}

void FileSystemHelper::Format(uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorsPerCluster)
{
    this->dev->Create();
    this->fs = new FileSystem(this->dev);
    this->fs->Format(volumeSize, bytesPerSector, sectorsPerCluster);
}

void FileSystemHelper::Read()
{
    this->dev->Open();
    this->fs = new FileSystem(this->dev);
    this->fs->Read();
}

void FileSystemHelper::ChangeDirectory(const std::string& path)
{
    Path newPath(this->path);
    newPath.Combine(path, false);

    // Check availability of the new path;
    Directory newDir(this->fs, newPath);

    Path newNormalizedPath(this->path);
    newNormalizedPath.Combine(path, true);

    this->path = std::move(newNormalizedPath);
}

Directory FileSystemHelper::GetCurrentDirectory() const
{
    Path tmp(this->path);
    return Directory(this->fs, std::move(tmp));
}
