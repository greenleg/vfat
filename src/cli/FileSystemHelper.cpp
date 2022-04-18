#include <iostream>
#include "../../include/FileDisk.h"
#include "../../include/cli/FileSystemHelper.h"

using namespace org::vfat::cli;

FileSystemHelper::FileSystemHelper(Device& device)
   : dev(device), fs(nullptr)
{
}

FileSystemHelper::~FileSystemHelper()
{
    this->dev.Close();
}

void FileSystemHelper::Format(uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorsPerCluster)
{
    this->dev.Create();
    this->fs = std::make_unique<FileSystem>(this->dev);
    this->fs->Format(volumeSize, bytesPerSector, sectorsPerCluster);
}

void FileSystemHelper::Read()
{
    this->dev.Open();
    this->fs = std::make_unique<FileSystem>(this->dev);
    this->fs->Read();
}

void FileSystemHelper::ChangeDirectory(const std::string& path)
{
    Path newPath(this->path);
    newPath.Combine(path, false);

    // Check availability of the new path;
    Directory newDir(this->fs.get(), newPath);

    Path newNormalizedPath(this->path);
    newNormalizedPath.Combine(path, true);

    this->path = std::move(newNormalizedPath);
}

Directory FileSystemHelper::GetCurrentDirectory() const
{
    Path tmp(this->path);
    return Directory(this->fs.get(), std::move(tmp));
}
