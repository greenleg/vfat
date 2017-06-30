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

bool filesys_close(/*in*/ struct filesys *fs)
{
    cchdir_write(fs->root, fs->dev);
    fat_write(fs->fat, fs->dev);
    vbr_write(fs->vbr, fs->dev);

    return true;
}

bool filesys_destruct(/*in*/ struct filesys *fs)
{
    cchdir_destruct(fs->root);
    fat_destruct(fs->fat);

    free(fs->root);
    free(fs->fat);
    free(fs->vbr);

    return true;
}

static int parse_path(/*in*/ const char *path, /*out*/ char *parts[256])
{
    // Skip leading '/'
    int n = 0;
    int len;
    int i = 0;
    int j;
    while (path[i] != '\0') {
        j = i + 1;
        while(path[j] != '\0' && path[j] != '/') {
            ++j;
        }

        if (j == i + 1) {
            i = j;
            continue;
        }

        len = j - i;

        parts[n] = malloc(sizeof(char) * len);
        memcpy(parts[n], &(path[i + 1]), len);
        parts[n][len - 1] = '\0';

        ++n;
        i = j;
    }

    return n;
}

bool filesys_mkdir(/*in*/ struct filesys *fs, /*in*/ const char *path)
{
    char *parts[256];
    int nparts = parse_path(path, parts);

    struct lfnde e;    
    struct cchdir *dir = fs->root;
    struct cchdir *subdir;

    for (int i = 0; i < nparts; ++i) {
        subdir = malloc(sizeof(struct cchdir));

        if (cchdir_findentry(dir, parts[i], &e)) {
            cchdir_getdir(fs->dev, fs->fat, &e, subdir);
        } else {
            cchdir_adddir(dir, parts[i], &e, subdir);
            cchdir_write(dir, fs->dev);
            if (i == nparts - 1) {
                cchdir_write(subdir, fs->dev);
            }
        }

        if (dir != fs->root) {
            cchdir_destruct(dir);
            free(dir);
        }

        dir = subdir;
    }

    if (dir != fs->root) {
        cchdir_destruct(dir);
        free(dir);
    }    

    for (int i = 0; i < nparts; ++i) {
        free(parts[i]);
    }

    return true;
}

struct vdir * filesys_opendir(/*in*/ struct filesys *fs, /*in*/ const char *path)
{
    char *parts[256];
    int nparts = parse_path(path, parts);

    struct lfnde e;
    struct cchdir *dir = fs->root;
    struct cchdir *subdir;

    bool notfound = false;
    for (int i = 0; i < nparts; ++i) {
        if (!cchdir_findentry(dir, parts[i], &e)) {
            notfound = true;
            break;
        }

        subdir = malloc(sizeof(struct cchdir));
        cchdir_getdir(fs->dev, fs->fat, &e, subdir);

        if (dir != fs->root) {
            cchdir_destruct(dir);
            free(dir);
        }

        dir = subdir;
    }

    /*if (dir != fs->root) {
        cchdir_destruct(dir);
        free(dir);
    }*/

    for (int i = 0; i < nparts; ++i) {
        free(parts[i]);
    }

    if (notfound) {
        return NULL;
    }

    struct vdir *vdir = malloc(sizeof(struct vdir));
    vdir->ccdir = dir;
    vdir->idx = 0;

    return vdir;
}

void filesys_closedir(/*in*/ struct filesys *fs, /*in*/ struct vdir *dir)
{
    if (dir->ccdir != fs->root) {
        cchdir_destruct(dir->ccdir);
        free(dir->ccdir);
    }

    free(dir);
}

bool filesys_readdir(/*in*/ struct vdir *dir, /*out*/ struct vdirent *entry)
{
    u32 n = alist_count(dir->ccdir->entries);
    if (dir->idx == n) {
        return false;
    }

    struct lfnde e;
    cchdir_getentry(dir->ccdir, dir->idx, &e);

    lfnde_getname(&e, entry->name);
    entry->isdir = lfnde_isdir(&e);
    entry->datalen = lfnde_getdatalen(&e);

    ++(dir->idx);

    return true;
}

struct vdir * filesys_getdir(/*in*/ struct filesys *fs, /*in*/ struct vdir *dir, /*in*/ const char *name)
{
    struct lfnde e;
    if (!cchdir_findentry(dir->ccdir, name, &e)) {
        return NULL;
    }

    struct cchdir *subccdir = malloc(sizeof(struct cchdir));
    cchdir_getdir(fs->dev, dir->ccdir->chain->fat, &e, subccdir);

    struct vdir *subdir = malloc(sizeof(struct vdir));
    subdir->ccdir = subccdir;
    subdir->idx = 0;

    return subdir;
}

struct vfile * filesys_fopen(/*in*/ struct filesys *fs,
                             /*in*/ const char *fname,
                             /*in*/ const char *mode)
{
    // Parse path to file
    return NULL;
}
