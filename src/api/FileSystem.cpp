#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include "../../include/api/FileSystem.h"
#include "../../include/api/Directory.h"
#include "../../include/Common.h"

using namespace org::vfat;
using namespace org::vfat::api;

//bool filesys_format(/*in*/ org::vfat::FileDisk *device,
//                    /*in*/ uint64_t volume_size,
//                    /*in*/ uint16_t bytes_per_sector,
//                    /*in*/ uint16_t sectors_per_cluster,
//                    /*out*/ struct filesys *fs)
//{
//    fs->device = device;
//    fs->bootSector = new BootSector();
//    fs->bootSector->Create(volume_size, bytes_per_sector, sectors_per_cluster);

//    fs->fat = new Fat(fs->bootSector);
//    fs->root = new ClusterChainDirectory();

//    fs->bootSector->Write(device);

//    fs->fat->Create();
//    fs->root->CreateRoot(fs->fat);

//    fs->fat->Write(device);
//    fs->root->Write(device);

//    return true;
//}

FileSystem::FileSystem(FileDisk *device)
{
    this->device = device;
}

void FileSystem::Format(uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorsPerCluster)
{
    this->bootSector = new BootSector();
    this->bootSector->Create(volumeSize, bytesPerSector, sectorsPerCluster);

    this->fat = new Fat(this->bootSector);
    this->root = new ClusterChainDirectory();

    this->fat->Create();
    this->root->CreateRoot(this->fat);

    this->bootSector->Write(device);
    this->fat->Write(device);
    this->root->Write(device);

    //this->currentDir = this->root;
}

void FileSystem::Open()
{
    this->bootSector = new BootSector();
    this->bootSector->Read(device);

    this->fat = new Fat(this->bootSector);
    this->fat->Read(device);

    this->root = new ClusterChainDirectory();
    this->root->ReadRoot(device, this->fat);

//    this->currentDir = this->root;
}

void FileSystem::Close()
{
    this->root->Write(this->device);
    this->fat->Write(this->device);
    this->bootSector->Write(this->device);
}

FileSystem::~FileSystem()
{
//    if (this->currentDir != this->root) {
//        delete this->currentDir;
//    }

    delete this->root;
    delete this->fat;
    delete this->bootSector;
}

//static int parse_path(/*in*/ const char *path, /*out*/ char *parts[256])
//{
//    // Skip leading '/'
//    int n = 0;
//    int len;
//    int i = 0;
//    int j;
//    while (path[i] != '\0') {
//        j = i + 1;
//        while(path[j] != '\0' && path[j] != '/') {
//            ++j;
//        }

//        if (j == i + 1) {
//            i = j;
//            continue;
//        }

//        len = j - i;

//        parts[n] = static_cast<char *>(malloc(sizeof(char) * len));
//        memcpy(parts[n], &(path[i + 1]), len);
//        parts[n][len - 1] = '\0';

//        ++n;
//        i = j;
//    }

//    return n;
//}

//void FileSystem::CreateDirectory(const char *name)
//{
//    string s(name);
//    this->CreateDirectory(s);
//}

//void FileSystem::ChangeDirectory(const char *path)
//{
//    string s(path);
//    this->ChangeDirectory(s);
//}

//struct vdir * filesys_opendir(/*in*/ struct filesys *fs, /*in*/ const char *path)
//{
//    char *parts[256];
//    int nparts = parse_path(path, parts);

//    DirectoryEntry *e;
//    ClusterChainDirectory *dir = fs->root;
//    ClusterChainDirectory *subdir;

//    bool notfound = false;
//    for (int i = 0; i < nparts; ++i) {
////        if (!cchdir_findentry(dir, parts[i], &e)) {
////            notfound = true;
////            break;
////        }

//        e = dir->FindEntry(parts[i]);
//        if (e == nullptr) {
//            notfound = true;
//            break;
//        }

//        subdir = ClusterChainDirectory::GetDirectory(fs->device, fs->fat, e);

//        if (dir != fs->root) {
//            delete dir;
//        }

//        dir = subdir;
//    }

//    /*if (dir != fs->root) {
//        cchdir_destruct(dir);
//        free(dir);
//    }*/

//    for (int i = 0; i < nparts; i++) {
//        free(parts[i]);
//    }

//    if (notfound) {
//        return NULL;
//    }

//    struct vdir *vdir = static_cast<struct vdir *>(malloc(sizeof(struct vdir)));
//    vdir->ccdir = dir;
//    vdir->idx = 0;

//    return vdir;
//}

//void filesys_closedir(/*in*/ struct filesys *fs, /*in*/ struct vdir *dir)
//{
//    if (dir->ccdir != fs->root) {
//        delete dir->ccdir;
//    }

//    free(dir);
//}

//bool filesys_readdir(/*in*/ struct vdir *dir, /*out*/ struct vdirent *entry)
//{
//    uint32_t n = dir->ccdir->GetEntries()->size();
//    if (dir->idx == n) {
//        return false;
//    }

//    DirectoryEntry *e = dir->ccdir->GetEntry(dir->idx);
//    e->GetName(entry->name);
//    entry->isdir = e->IsDir();
//    entry->datalen = e->GetDataLength();

//    ++(dir->idx);

//    return true;
//}

//struct vdir * filesys_getdir(/*in*/ struct filesys *fs, /*in*/ struct vdir *dir, /*in*/ const char *name)
//{
////    DirectoryEntry e;
////    if (!cchdir_findentry(dir->ccdir, name, &e)) {
////        return NULL;
////    }

//    DirectoryEntry *e = dir->ccdir->FindEntry(name);
//    if (e == nullptr) {
//        return nullptr;
//    }

//    ClusterChainDirectory *subccdir = ClusterChainDirectory::GetDirectory(fs->device, fs->fat, e);
//    struct vdir *subdir = static_cast<struct vdir *>(malloc(sizeof(struct vdir)));
//    subdir->ccdir = subccdir;
//    subdir->idx = 0;

//    return subdir;
//}

//struct vfile * filesys_fopen(/*in*/ struct filesys *fs,
//                             /*in*/ const char *fname,
//                             /*in*/ const char *mode)
//{
//    // Parse path to file
//    return NULL;
//}