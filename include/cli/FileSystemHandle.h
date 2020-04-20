#ifndef VFAT_FILESYSTEMHANDLE_H
#define VFAT_FILESYSTEMHANDLE_H

#include "../api/FileSystem.h"
#include "../api/Directory.h"

using namespace org::vfat::api;

namespace org::vfat::cli
{
    class FileSystemHandle
    {
    private:
        FileDisk *dev = nullptr;
        FileSystem *fs = nullptr;
        Directory *dir = nullptr;

    public:
        FileSystemHandle(string deviceName);
        ~FileSystemHandle();

        void Format(uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorsPerCluster);
        void Read();

        void ChangeDirectory(string path);

        FileSystem* GetFileSystem() const { return this->fs; }
        Directory* GetCurrentDirectory() const { return this->dir; }
    };
}

#endif // VFAT_FILESYSTEMHANDLE_H
