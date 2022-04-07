#ifndef VFAT_FILESYSTEMHELPER_H
#define VFAT_FILESYSTEMHELPER_H

#include <memory>
#include "../api/FileSystem.h"
#include "../api/Directory.h"

using namespace org::vfat::api;

namespace org::vfat::cli
{
    /** 
     * FileSystem CLI helper 
     */
    class FileSystemHelper
    {
    private:
        Device *dev = nullptr;
        FileSystem *fs = nullptr;
        Path path;

    public:
        FileSystemHelper(const std::string& deviceName);
        ~FileSystemHelper();

        void Format(uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorsPerCluster);
        void Read();

        void ChangeDirectory(const std::string& path);

        FileSystem* GetFileSystem() const { return this->fs; }
        Path GetCurrentPath() const { return this->path; }
        Directory* GetCurrentDirectory() const;
    };
}

#endif // VFAT_FILESYSTEMHELPER_H
