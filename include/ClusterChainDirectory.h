#ifndef VFAT_CCHDIR_H
#define VFAT_CCHDIR_H

#include <stdbool.h>

#include "ClusterChain.h"
#include "DirectoryEntry.h"
#include "ClusterChainFile.h"
#include "BootSector.h"

using namespace org::vfat;

//struct cchdir
//{
//    struct ClusterChain *chain;
//    struct alist *entries;
//    uint32_t capacity;
//    bool root;
//    bool dirty;
//};

//void cchdir_formatdev(/*in*/ FileDisk * device,
//                      /*in*/ uint64_t vol_size,
//                      /*in*/ uint16_t bytes_per_sect,
//                      /*in*/ uint16_t sect_per_clus);

//void cchdir_write(struct cchdir* dir, FileDisk *device);
//void cchdir_read(FileDisk *device, struct fat *fat, struct cchdir* dir, uint32_t first_cluster, bool root);

//void cchdir_create(struct cch *cc, struct cchdir *dir);
//void cchdir_createroot(Fat *fat, struct cchdir *dir);
//void cchdir_readroot(FileDisk *device, Fat *fat, struct cchdir *dir);
//void cchdir_addentry(struct cchdir *dir, DirectoryEntry *e);
//void cchdir_getentry(struct cchdir *dir, uint32_t idx, DirectoryEntry *e);
//bool cchdir_findentry(/*in*/ struct cchdir *dir, /*out*/ const char *name, /*out*/ DirectoryEntry *e);
//void cchdir_removeentry(struct cchdir *dir, uint32_t idx);
//bool cchdir_removedir(/*in*/ struct cchdir *dir, /*in*/ const char *name);
//bool cchdir_createsubdir(/*in*/ struct cchdir *parentdir, /*out*/ struct cchdir *subdir, /*out*/ DirectoryEntry* subde);

//bool cchdir_adddir(/*in*/ struct cchdir *dir,
//                   /*in*/ const char *name,
//                   /*out*/ DirectoryEntry *subde,
//                   /*out*/ struct cchdir *subdir);

//bool cchdir_addfile(/*in*/ struct cchdir *dir, /*in*/ const char *name, /*out*/ DirectoryEntry *e);

//void cchdir_getfile(/*in*/ struct cchdir *dir,
//                    /*in*/ DirectoryEntry *e,
//                    /*out*/ struct cchfile *file);

//bool cchdir_move(/*in*/ FileDisk *device,
//                 /*in*/ struct cchdir *src,
//                 /*in*/ DirectoryEntry *e,
//                 /*in*/ struct cchdir *dst,
//                 /*in*/ const char *new_name);

//bool cchdir_setname(/*in*/ FileDisk *device,
//                    /*in*/ struct cchdir *dir,
//                    /*in*/ DirectoryEntry *e,
//                    /*in*/ const char *name);

//bool cchdir_getdir(/*in*/ FileDisk *device,
//                   /*in*/ Fat *fat,
//                   /*in*/ DirectoryEntry *e,
//                   /*out*/ struct cchdir *dir);

//bool cchdir_copyfile(/*in*/ FileDisk *device,
//                     /*in*/ struct cchdir *src,
//                     /*in*/ DirectoryEntry *e,
//                     /*in*/ struct cchdir *dst);

//bool cchdir_copydir(/*in*/ FileDisk *device,
//                    /*in*/ struct cchdir *src,
//                    /*in*/ DirectoryEntry *e,
//                    /*in*/ struct cchdir *dst);

//void cchdir_destruct(struct cchdir *dir);

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

        bool RemoveDirectory(const char *name);
        bool RemoveFile(const char *name);

        DirectoryEntry* AddDirectory(const char *name, FileDisk *device);
        DirectoryEntry* AddFile(const char *name, FileDisk *device);

        static ClusterChainDirectory* GetDirectory(FileDisk *device, Fat *fat, DirectoryEntry *e);
        static ClusterChainFile* GetFile(Fat *fat, DirectoryEntry *e);

        void SetName(FileDisk *device, DirectoryEntry *e, const char *name);
        void Move(FileDisk *device, DirectoryEntry *e, ClusterChainDirectory *dest, const char *newName);

        void CopyFile(FileDisk *device, DirectoryEntry *e, ClusterChainDirectory *dest) const;
        void CopyDirectory(FileDisk *device, DirectoryEntry *e, ClusterChainDirectory *dest) const;

        std::vector<DirectoryEntry *> * GetEntries() const { return this->entries; }
    };
}

#endif /* VFAT_CCHDIR_H */
