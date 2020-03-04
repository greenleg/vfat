#ifndef VFAT_CCHDIR_H
#define VFAT_CCHDIR_H

#include <stdbool.h>

#include "common.h"
#include "alist.h"
#include "cch.h"
#include "lfnde.h"
#include "cchfile.h"

struct cchdir
{
    struct cch *chain;
    struct alist *entries;
    u32 capacity;
    bool root;
    bool dirty;
};

void cchdir_formatdev(/*in*/ org::vfat::FileDisk * device,
                      /*in*/ u64 vol_size,
                      /*in*/ u16 bytes_per_sect,
                      /*in*/ u16 sect_per_clus);

void cchdir_write(struct cchdir* dir, org::vfat::FileDisk *device);
void cchdir_read(org::vfat::FileDisk *device, struct fat *fat, struct cchdir* dir, u32 first_cluster, bool root);

void cchdir_create(struct cch *cc, struct cchdir *dir);
void cchdir_createroot(struct fat *fat, struct cchdir *dir);
void cchdir_readroot(org::vfat::FileDisk *device, struct fat *fat, struct cchdir *dir);
void cchdir_addentry(struct cchdir *dir, struct lfnde *e);
void cchdir_getentry(struct cchdir *dir, u32 idx, struct lfnde *e);
bool cchdir_findentry(/*in*/ struct cchdir *dir, /*out*/ const char *name, /*out*/ struct lfnde *e);
void cchdir_removeentry(struct cchdir *dir, u32 idx);
bool cchdir_removedir(/*in*/ struct cchdir *dir, /*in*/ const char *name);
bool cchdir_createsubdir(/*in*/ struct cchdir *parentdir, /*out*/ struct cchdir *subdir, /*out*/ struct lfnde* subde);

bool cchdir_adddir(/*in*/ struct cchdir *dir,
                   /*in*/ const char *name,
                   /*out*/ struct lfnde *subde,
                   /*out*/ struct cchdir *subdir);

bool cchdir_addfile(/*in*/ struct cchdir *dir, /*in*/ const char *name, /*out*/ struct lfnde *e);

void cchdir_getfile(/*in*/ struct cchdir *dir,
                    /*in*/ struct lfnde *e,
                    /*out*/ struct cchfile *file);

bool cchdir_move(/*in*/ org::vfat::FileDisk *device,
                 /*in*/ struct cchdir *src,
                 /*in*/ struct lfnde *e,
                 /*in*/ struct cchdir *dst,
                 /*in*/ const char *new_name);

bool cchdir_setname(/*in*/ org::vfat::FileDisk *device,
                    /*in*/ struct cchdir *dir,
                    /*in*/ struct lfnde *e,
                    /*in*/ const char *name);

bool cchdir_getdir(/*in*/ org::vfat::FileDisk *device,
                   /*in*/ struct fat *fat,
                   /*in*/ struct lfnde *e,
                   /*out*/ struct cchdir *dir);

bool cchdir_copyfile(/*in*/ org::vfat::FileDisk *device,
                     /*in*/ struct cchdir *src,
                     /*in*/ struct lfnde *e,
                     /*in*/ struct cchdir *dst);

bool cchdir_copydir(/*in*/ org::vfat::FileDisk *device,
                    /*in*/ struct cchdir *src,
                    /*in*/ struct lfnde *e,
                    /*in*/ struct cchdir *dst);

void cchdir_destruct(struct cchdir *dir);

#endif /* VFAT_CCHDIR_H */