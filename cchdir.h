#ifndef VFAT_CCHDIR_H
#define VFAT_CCHDIR_H

#include "common.h"
#include "arraylist.h"
#include "clusterchain.h"
#include "filedirentry.h"

struct cchdir
{
    struct cch *chain;
    struct alist *entries;
    u32 capacity;
    u8 is_root;
    u8 dirty;
};

void cchdir_write(struct cchdir* dir, struct fdisk *disk);
void cchdir_read(struct fdisk *disk, struct cchdir* dir);

void cchdir_createroot(struct fat *fat, struct cchdir *dir);
void cchdir_add(struct cchdir *dir, struct lfnde *e);
void cchdir_remove(struct lfnde *e);
void cchdir_createsubdir(struct fat* fat, struct lfnde *e);

#endif /* VFAT_CCHDIR_H */
