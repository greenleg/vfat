#include "filesys.h"

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

static int parse_path(/*in*/ const char *path, /*out*/ char *parts[256])
{
    // Skip leading '/'
    int cnt = 0;
    int len;
    int i = 0;
    int j;
    while (path[i] != '\0') {
        j = i + 1;
        while(path[j] != '\0' && path[j] != '/') {
            ++j;
        }

        len = j - i;

        parts[cnt] = malloc(sizeof(char) * len);
        memcpy(parts[cnt], &(path[i + 1]), len);
        parts[cnt][len - 1] = '\0';

        ++cnt;
        i = j;
    }

    return cnt;
}

bool filesys_mkdir(/*in*/ struct filesys *fs, /*in*/ const char *path)
{
    char *parts[256];
    int cnt = parse_path(path, parts);

    struct lfnde e;
    //struct lfnde e1;
    struct cchdir *dir = fs->root;
    struct cchdir *subdir = malloc(sizeof(struct cchdir));

    for (int i = 0; i < cnt; ++i) {
        if (cchdir_findentry(dir, parts[i], &e)) {
            cchdir_getdir(fs->dev, fs->fat, &e, subdir);
        } else {
            cchdir_adddir(dir, parts[i], &e, subdir);
            cchdir_write(dir, fs->dev);
            if (i == cnt - 1) {
                cchdir_write(subdir, fs->dev);
            }
        }

        if (dir != fs->root) {
            cchdir_destruct(dir);
            free(dir);
        }

        dir = subdir;
        subdir = malloc(sizeof(struct cchdir));
    }

    if (dir != fs->root) {
        cchdir_destruct(dir);
        free(dir);
    }

    free(subdir);

    for (int i = 0; i < cnt; ++i) {
        free(parts[i]);
    }

    return true;
}

struct vfile * filesys_fopen(/*in*/ struct filesys *fs,
                             /*in*/ const char *fname,
                             /*in*/ const char *mode)
{
    // Parse path to file
    return NULL;
}
