#ifndef VFAT_LFNDE_H
#define VFAT_LFNDE_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <vector>
#include <memory>

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
    
    struct DirectoryEntryImpl
    {
        uint16_t attributes;
        uint32_t created;
        uint32_t lastModified;
        uint32_t lastAccessed;
        uint8_t nameLength;
        uint32_t firstCluster;
        uint32_t dataLength;
        std::vector<FileNameDirectoryEntry> fndeList;
        
        DirectoryEntryImpl()
          : attributes(0),
            created(),
            lastModified(0),
            lastAccessed(0),
            nameLength(0),
            firstCluster(0),
            dataLength(0)
        {
        }
        
        DirectoryEntryImpl(const DirectoryEntryImpl& other)
          : attributes(other.attributes),
            created(other.created),
            lastModified(other.lastModified),
            lastAccessed(other.lastAccessed),
            nameLength(other.nameLength),
            firstCluster(other.firstCluster),
            dataLength(other.dataLength),
            fndeList(other.fndeList)
        {
        }
        
        DirectoryEntryImpl(DirectoryEntryImpl&& other)
          : attributes(std::exchange(other.attributes, 0)),
            created(std::exchange(other.created, 0)),
            lastModified(std::exchange(other.lastModified, 0)),
            lastAccessed(std::exchange(other.lastAccessed, 0)),
            nameLength(std::exchange(other.nameLength, 0)),
            firstCluster(std::exchange(other.firstCluster, 0)),
            dataLength(std::exchange(other.dataLength, 0)),
            fndeList(std::move(other.fndeList))
        {
        }
        
        DirectoryEntryImpl& operator=(const DirectoryEntryImpl& other)
        {
            if (this != &other) {
                attributes = other.attributes;
                created = other.created;
                lastModified = other.lastModified;
                lastAccessed = other.lastAccessed;
                nameLength = other.nameLength;
                firstCluster = other.firstCluster;
                dataLength = other.dataLength;
                fndeList = other.fndeList;
            }
            return *this;
        }
        
        DirectoryEntryImpl& operator=(DirectoryEntryImpl&& other)
        {
            if (this != &other) {
                attributes = std::exchange(other.attributes, 0);
                created = std::exchange(other.created, 0);
                lastModified = std::exchange(other.lastModified, 0);
                lastAccessed = std::exchange(other.lastAccessed, 0);
                nameLength = std::exchange(other.nameLength, 0);
                firstCluster = std::exchange(other.firstCluster, 0);
                dataLength = std::exchange(other.dataLength, 0);
                fndeList = std::move(other.fndeList);
            }
            return *this;
        }
        
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
    };

    class DirectoryEntry
    {
    private:
        std::unique_ptr<DirectoryEntryImpl> pImpl;

    public:
        DirectoryEntry();
        DirectoryEntry(const DirectoryEntry& other);
        DirectoryEntry(DirectoryEntry&& other);
        DirectoryEntry& operator=(const DirectoryEntry& other);
        DirectoryEntry& operator=(DirectoryEntry&& other);
        ~DirectoryEntry();

        void Read(uint8_t *buffer) { pImpl->Read(buffer); }        
        void Write(uint8_t *buffer) const { pImpl->Write(buffer); }
        uint32_t GetDataLength() const { return pImpl->GetDataLength(); }
        void SetDataLength(uint32_t val) { pImpl->SetDataLength(val); }
        uint32_t GetStartCluster() const { return pImpl->GetStartCluster(); }
        void SetStartCluster(uint32_t val) { pImpl->SetStartCluster(val); }
        void GetName(/*out*/ char *name) const { pImpl->GetName(name); }
        void SetName(const char *name) { pImpl->SetName(name); }
        time_t GetLastModifiedTime() const { return pImpl->GetLastModifiedTime(); }
        void SetLastModifiedTime(time_t val) { pImpl->SetLastModifiedTime(val); }
        time_t GetCreatedTime() const { return pImpl->GetCreatedTime(); }
        void SetCreatedTime(time_t time) { pImpl->SetCreatedTime(time); }
        bool IsDir() const { return pImpl->IsDir(); }
        bool IsFile() const { return pImpl->IsFile(); }
        void SetIsDir(bool val) { pImpl->SetIsDir(val); }
        uint16_t GetFat32EntryCount() const { return pImpl->GetFat32EntryCount(); }
    };
}

#endif /* VFAT_LFNDE_H */
