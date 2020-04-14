#ifndef VFAT_FILE_H
#define VFAT_FILE_H

#include <string>
#include "../DirectoryEntry.h"
#include "FileSystem.h"

using namespace std;

namespace org::vfat::api
{
    class File
    {
    private:
        FileSystem *fs;
        DirectoryEntry *entry;

    public:
        File(FileSystem *fs, DirectoryEntry *e);
        ~File();
        string GetName() const;
        uint32_t GetSize() const;
        uint32_t Read(uint32_t offset, uint32_t nbytes, uint8_t *buffer) const;
        void Write(uint32_t offset, uint32_t nbytes, uint8_t *buffer) const;
        string ReadText(uint32_t offset, uint32_t nchars) const;
        void WriteText(string s, uint32_t offset) const;
    };
}

#endif // VFAT_FILE_H