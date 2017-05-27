#ifndef VFAT_CCHDIR_H
#define VFAT_CCHDIR_H

#include <stdbool.h>

#include "common.h"
#include "alist.h"
#include "cch.h"
#include "lfnde.h"

struct cchdir
{
    struct cch *chain;
    struct alist *entries;
    u32 capacity;
    bool root;
    bool dirty;
};

void cchdir_formatdev(/*in*/ struct fdisk *dev,
                      /*in*/ u64 vol_size,
                      /*in*/ u16 bytes_per_sect,
                      /*in*/ u16 sect_per_clus);

void cchdir_write(struct cchdir* dir, struct fdisk *disk);
void cchdir_read(struct fdisk *disk, struct fat *fat, struct cchdir* dir, u32 first_cluster, bool root);

void cchdir_create(struct cch *cc, struct cchdir *dir);
void cchdir_createroot(struct fat *fat, struct cchdir *dir);
void cchdir_readroot(struct fdisk *disk, struct fat *fat, struct cchdir *dir);
void cchdir_addentry(struct cchdir *dir, struct lfnde *e);
void cchdir_getentry(struct cchdir *dir, u32 idx, struct lfnde *e);
bool cchdir_findentry(/*in*/ struct cchdir *dir, /*out*/ const char *name, /*out*/ struct lfnde *e);
void cchdir_removeentry(struct cchdir *dir, u32 idx);
bool cchdir_removedir(/*in*/ struct cchdir *dir, /*in*/ const char *name);
bool cchdir_createsubdir(/*in*/ struct cchdir *parentdir, /*out*/ struct cchdir *subdir, /*out*/ struct lfnde* subde);
bool cchdir_adddir(/*in*/ struct cchdir *dir, /*in*/ const char *name, /*out*/ struct lfnde *e);
void cchdir_destruct(struct cchdir *dir);

#endif /* VFAT_CCHDIR_H */
