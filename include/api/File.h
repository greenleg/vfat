#ifndef VFAT_FILE_H
#define VFAT_FILE_H

#include <string>
#include "../DirectoryEntry.h"
#include "FileSystem.h"
#include "DirectoryItem.h"
#include "Path.h"

using namespace std;

namespace org::vfat::api
{
    class File : public DirectoryItem
    {
    private:
        FileSystem *fs;
        ClusterChainDirectory *parentCchDir;
        DirectoryEntry *entry;
        Path path;
        
        void Init();
        void Cleanup();

    public:  
        File();      
        File(FileSystem *fs, Path& path);
        File(FileSystem *fs, Path&& path);
        File(const File& other);
        File(File&& other);
        File& operator=(const File& other);
        File& operator=(File&& other);
        ~File();
        uint32_t GetSize() const;
        uint32_t Read(uint32_t offset, uint32_t nbytes, uint8_t *buffer) const;
        void Write(uint32_t offset, uint32_t nbytes, uint8_t *buffer) const;
        std::string ReadText(uint32_t offset, uint32_t nchars) const;
        void WriteText(const std::string& s, uint32_t offset) const;

        /* `DirectoryItem` implementation */
        std::string GetName() const;
        tm* GetCreatedTime() const;
        tm* GetModifiedTime() const;
    };
}

#endif // VFAT_FILE_H


