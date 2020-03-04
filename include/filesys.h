#ifndef VFAT_FILESYS_H
#define VFAT_FILESYS_H

#include "FileDisk.h"
#include "fat.h"
#include "cchdir.h"

struct filesys
{
    org::vfat::FileDisk *device;
    struct vbr *vbr;
    struct fat *fat;
    struct cchdir *root;
};

struct vdir
{
    struct cchdir *ccdir;
    u32 idx;
};

struct vdirent
{
    char name[256];
    bool isdir;
    u64 datalen;
};

struct vfile
{
    struct cchfile *file;
};

bool filesys_format(/*in*/ org::vfat::FileDisk *device,
                    /*in*/ u64 volume_size,
                    /*in*/ u16 bytes_per_sector,
                    /*in*/ u16 sectors_per_cluster,
                    /*out*/ struct filesys *fs);

bool filesys_open(/*in*/ org::vfat::FileDisk *device, /*out*/ struct filesys *fs);
bool filesys_close(/*in*/ struct filesys *fs);
bool filesys_destruct(/*in*/ struct filesys *fs);

bool filesys_mkdir(/*in*/ struct filesys *fs, /*in*/ const char *path);
struct vdir * filesys_opendir(/*in*/ struct filesys *fs, /*in*/ const char *path);
struct vdir * filesys_getdir(/*in*/ struct filesys *fs, /*in*/ struct vdir *dir, /*in*/ const char *name);
void filesys_closedir(/*in*/ struct filesys *fs, /*in*/ struct vdir *dir);
bool filesys_readdir(/*in*/ struct vdir *dir, /*out*/ struct vdirent *entry);

struct vfile * filesys_fopen(/*in*/ struct filesys *fs,
                             /*in*/ const char *fname,
                             /*in*/ const char *mode);

#endif /* VFAT_FILESYS_H */
