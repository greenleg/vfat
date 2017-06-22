#include "vfstdio.h"

bool filesys_format(/*in*/ struct fdisk *dev,
                    /*in*/ u64 volume_size,
                    /*in*/ u16 bytes_per_sector,
                    /*in*/ u16 sectors_per_cluster,
                    /*out*/ struct filesys *fs)
{
    fs->dev = dev;
    fs->vbr = malloc(sizeof(struct vbr));
    fs->fat = malloc(sizeof(struct fat));
    fs->root = malloc(sizeof(struct cchdir));

    vbr_create(fs->vbr, volume_size, bytes_per_sector, sectors_per_cluster);
    vbr_write(fs->vbr, dev);

    fat_create(fs->vbr, fs->fat);
    cchdir_createroot(fs->fat, fs->root);

    fat_write(fs->fat, dev);
    cchdir_write(fs->root, dev);

    return true;
}

bool filesys_open(/*in*/ struct fdisk *dev, /*out*/ struct filesys *fs)
{
    fs->dev = dev;
    fs->vbr = malloc(sizeof(struct vbr));
    fs->fat = malloc(sizeof(struct fat));
    fs->root = malloc(sizeof(struct cchdir));

    vbr_read(dev, fs->vbr);
    fat_read(dev, fs->vbr, fs->fat);
    cchdir_readroot(dev, fs->fat, fs->root);

    return true;
}

bool filesys_destruct(/*in*/ struct filesys *fs)
{
    cchdir_destruct(fs->root);
    fat_destruct(fs->fat);

    return true;
}

struct vfile * vf_fopen(/*in*/ struct filesys *fs,
                         /*in*/ const char *fname,
                         /*in*/ const char *mode)
{
    // Parse path to file

}
