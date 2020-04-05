#ifndef VFAT_FILESYS_H
#define VFAT_FILESYS_H

#include <string>
#include <vector>

#include "../FileDisk.h"
#include "../Fat.h"
#include "../ClusterChainDirectory.h"

//struct filesys
//{
//    org::vfat::FileDisk *device;
//    org::vfat::BootSector *bootSector;
//    Fat *fat;
//    org::vfat::ClusterChainDirectory *root;
//};

struct vdir
{
    org::vfat::ClusterChainDirectory *ccdir;
    uint32_t idx;
};

struct vdirent
{
    char name[256];
    bool isdir;
    uint64_t datalen;
};

struct vfile
{
    struct cchfile *file;
};

//bool filesys_format(/*in*/ org::vfat::FileDisk *device,
//                    /*in*/ uint64_t volume_size,
//                    /*in*/ uint16_t bytes_per_sector,
//                    /*in*/ uint16_t sectors_per_cluster,
//                    /*out*/ struct filesys *fs);

//bool filesys_open(/*in*/ org::vfat::FileDisk *device, /*out*/ struct filesys *fs);
//bool filesys_close(/*in*/ struct filesys *fs);
//bool filesys_destruct(/*in*/ struct filesys *fs);

//bool filesys_mkdir(/*in*/ struct filesys *fs, /*in*/ const char *path);
//struct vdir * filesys_opendir(/*in*/ struct filesys *fs, /*in*/ const char *path);
//struct vdir * filesys_getdir(/*in*/ struct filesys *fs, /*in*/ struct vdir *dir, /*in*/ const char *name);
//void filesys_closedir(/*in*/ struct filesys *fs, /*in*/ struct vdir *dir);
//bool filesys_readdir(/*in*/ struct vdir *dir, /*out*/ struct vdirent *entry);

//struct vfile * filesys_fopen(/*in*/ struct filesys *fs,
//                             /*in*/ const char *fname,
//                             /*in*/ const char *mode);

using namespace org::vfat;

namespace org::vfat::api
{
    class FileSystem
    {
    private:
        FileDisk *device;
        BootSector *bootSector;
        Fat *fat;
        ClusterChainDirectory *root;
        ClusterChainDirectory *currentDir;

    public:
        FileSystem(FileDisk *device);
        ~FileSystem();
        void Format(uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorsPerCluster);
        void Open();
        void Close();

        void ChangeDirectory(std::string& path);
        void CreateDirectory(std::string& name);
    };
}

#endif /* VFAT_FILESYS_H */
