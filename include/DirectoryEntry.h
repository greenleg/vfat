#ifndef VFAT_LFNDE_H
#define VFAT_LFNDE_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <vector>

#include "FileDisk.h"

#define FAT_DIR_ENTRY_SIZE 32

#define BASE_DIR_ENTRY 0x01
#define FILENAME_DIR_ENTRY 0x02
#define NO_DIR_ENTRY 0x00

#define FNDE_NAME_LENGTH 30

namespace org::vfat
{
    struct FileNameDirectoryEntry
    {
        char nameBuffer[FNDE_NAME_LENGTH];
    };

    class DirectoryEntry
    {
    private:
        uint16_t attributes;
        uint32_t created;
        uint32_t lastModified;
        uint32_t lastAccessed;
        uint8_t nameLength;
        uint32_t firstCluster;
        uint32_t dataLength;
        std::vector<FileNameDirectoryEntry *>* fndeList;

    public:
        DirectoryEntry();
        ~DirectoryEntry();
        //void Create();
        void Read(uint8_t *buffer);
        void Write(uint8_t *buffer) const;
        uint32_t GetDataLength() const;
        void SetDataLength(uint32_t val);
        uint32_t GetStartCluster() const;
        void SetStartCluster(uint32_t val);
        void GetName(/*out*/ char *name) const;
        void SetName(const char *name);
        time_t GetLastModifiedTime() const;
        void SetLastModifiedTime(time_t val);
        time_t GetCreatedTime() const;
        void SetCreatedTime(time_t time);
        bool IsDir() const;
        bool IsFile() const;
        void SetIsDir(bool val);
        uint16_t GetFat32EntryCount() const;
        DirectoryEntry* Clone() const;
    };
}

#endif /* VFAT_LFNDE_H */
