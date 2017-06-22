#ifndef VFAT_STDIO_H
#define VFAT_STDIO_H

#include "fdisk.h"
#include "fat.h"
#include "cchdir.h"

struct filesys
{
    struct fdisk *dev;
    struct vbr *vbr;
    struct fat *fat;
    struct cchdir *root;
};

bool filesys_format(/*in*/ struct fdisk *dev,
                    /*in*/ u64 volume_size,
                    /*in*/ u16 bytes_per_sector,
                    /*in*/ u16 sectors_per_cluster,
                    /*out*/ struct filesys *fs);

bool filesys_open(/*in*/ struct fdisk *dev, /*out*/ struct filesys *fs);

bool filesys_destruct(/*in*/ struct filesys *fs);

struct vfile
{
    struct cchfile *file;
};

struct vfile * vf_fopen(/*in*/ struct filesys *fs,
                        /*in*/ const char *fname,
                        /*in*/ const char *mode);

#endif /* VFAT_STDIO_H */
