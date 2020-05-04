#ifndef VFAT_CCHDIR_H
#define VFAT_CCHDIR_H

#include <stdbool.h>

#include "ClusterChain.h"
#include "DirectoryEntry.h"
#include "ClusterChainFile.h"
#include "BootSector.h"

using namespace org::vfat;

namespace org::vfat
{
    class ClusterChainDirectory
    {
    private:
        ClusterChain *chain;
        std::vector<DirectoryEntry *> *entries;
        uint32_t capacity;
        bool isRoot;
        bool isDirty;

    private:
        void ReadEntries(uint8_t *buffer);
        uint32_t WriteEntries(uint8_t *buffer, uint32_t bufferSize) const;
        uint32_t GetFat32EntryCount() const;
        void ChangeSize(uint32_t fat32EntryCount);
        int32_t FindEntryIndex(const char *name);
        void CheckUniqueName(const char *name);

    public:
        ~ClusterChainDirectory();
        void Write(FileDisk *device) const;
        void Read(FileDisk *device, Fat *fat, uint32_t firstCluster, bool isRoot);
        void ReadRoot(FileDisk *device, Fat *fat);
        void Create(ClusterChain *cc);
        void CreateRoot(Fat *fat);
        void AddEntry(DirectoryEntry *e);
        DirectoryEntry* GetEntry(uint32_t index) const;
        DirectoryEntry* FindEntry(const char *name) const;
        void RemoveEntry(uint32_t index);
        static void FormatDevice(FileDisk * device, uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorPerCluster);

        void RemoveDirectory(const char *name, FileDisk *device);
        void RemoveFile(const char *name, FileDisk *device);
        void RemoveDirectory(uint32_t index, FileDisk *device);
        void RemoveFile(uint32_t index, FileDisk *device);

        DirectoryEntry* AddDirectory(const char *name, FileDisk *device);
        DirectoryEntry* AddFile(const char *name, FileDisk *device);

        static ClusterChainDirectory* GetDirectory(FileDisk *device, Fat *fat, DirectoryEntry *e);
        static ClusterChainFile* GetFile(Fat *fat, DirectoryEntry *e);

        void SetName(FileDisk *device, DirectoryEntry *e, const char *name);
        void Move(FileDisk *device, DirectoryEntry *e, ClusterChainDirectory *dest, const char *newName);

        void CopyFile(FileDisk *device, DirectoryEntry *e, ClusterChainDirectory *dest) const;
        void CopyFile(FileDisk *device, DirectoryEntry *e, ClusterChainDirectory *dest, const char *newName) const;
        void CopyDirectory(FileDisk *device, DirectoryEntry *e, ClusterChainDirectory *dest) const;
        void CopyDirectory(FileDisk *device, DirectoryEntry *e, ClusterChainDirectory *dest, const char *newName) const;

        std::vector<DirectoryEntry *> * GetEntries() const { return this->entries; }
        uint32_t GetStartCluster() const { return this->chain->GetStartCluster(); }
    };
}

#endif /* VFAT_CCHDIR_H */
