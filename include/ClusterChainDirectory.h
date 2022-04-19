#ifndef VFAT_CCHDIR_H
#define VFAT_CCHDIR_H

#include <stdbool.h>

#include "ClusterChain.h"
#include "DirectoryEntry.h"
#include "ClusterChainFile.h"
#include "BootSector.h"

namespace org::vfat
{
    class ClusterChainDirectory
    {
    private:
        ClusterChain chain;
        std::vector<DirectoryEntry> entries;
        uint32_t capacity;
        bool isRoot;
        bool isDirty;

    private:
        void ReadEntries(uint8_t *buffer);
        uint32_t WriteEntries(uint8_t *buffer, uint32_t bufferSize) const;
        uint32_t GetFat32EntryCount() const;
        void ChangeSize(Fat& fat, uint32_t fat32EntryCount);
        void CheckUniqueName(const std::string& name);
        void Cleanup();
        
    public:
        ClusterChainDirectory();
        ClusterChainDirectory(const ClusterChainDirectory& other);
        ClusterChainDirectory(ClusterChainDirectory&& other);
        ClusterChainDirectory& operator=(const ClusterChainDirectory& other);
        ClusterChainDirectory& operator=(ClusterChainDirectory&& other);
        ~ClusterChainDirectory();
        void Write(Device& device, Fat& fat);
        void Read(const Device& device, const Fat& fat, uint32_t firstCluster, bool isRoot);
        void ReadRoot(const Device& device, const Fat& fat);
        void Create(ClusterChain& cc, const Fat& fat);
        void CreateRoot(Fat& fat);
        DirectoryEntry& AddEntry(DirectoryEntry& e, Fat& fat);
        DirectoryEntry& GetEntry(uint32_t index);
        DirectoryEntry& FindEntry(const std::string& name);
        const DirectoryEntry& FindEntry(const std::string& name) const;
        int32_t FindEntryIndex(const std::string& name);
        void RemoveEntry(Fat& fat, uint32_t index);
        static void FormatDevice(Device& device, uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorPerCluster);

        void RemoveDirectory(const std::string& name, Device& device, Fat& fat);
        void RemoveFile(const std::string& name, Device& device, Fat& fat);
        void RemoveDirectory(uint32_t index, Device& device, Fat& fat);
        void RemoveFile(uint32_t index, Device& device, Fat& fat);

        DirectoryEntry& AddDirectory(const std::string& name, Device& device, Fat& fat);
        DirectoryEntry& AddFile(const std::string& name, Device& device, Fat& fat);

        static ClusterChainDirectory GetDirectory(const Device& device, const Fat& fat, const DirectoryEntry& e);
        static ClusterChainFile GetFile(const DirectoryEntry& e);

        void SetName(Device& device, Fat& fat, DirectoryEntry& e, const std::string& name);
        void Move(Device& device, Fat& fat, DirectoryEntry& e, ClusterChainDirectory& dest, const std::string& newName);

        ClusterChainFile CopyFile(Device& device, Fat& fat, DirectoryEntry& e, ClusterChainDirectory& dest) const;
        ClusterChainFile CopyFile(Device& device, Fat& fat, DirectoryEntry& e, ClusterChainDirectory& dest, const std::string& newName) const;
        void CopyDirectory(Device& device, Fat& fat, DirectoryEntry& e, ClusterChainDirectory& dest) const;
        void CopyDirectory(Device& device, Fat& fat, DirectoryEntry& e, ClusterChainDirectory& dest, const std::string& newName) const;

        std::vector<DirectoryEntry> GetEntries() const { return this->entries; }
        uint32_t GetStartCluster() const { return this->chain.GetStartCluster(); }
    };
}

#endif /* VFAT_CCHDIR_H */
