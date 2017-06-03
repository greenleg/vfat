#include <assert.h>

#include "alist.h"
#include "fat.h"
#include "cchdir.h"
#include "cchfile.h"
#include "lfnde.h"

static bool check_unique_name(/*in*/ struct cchdir *dir, /*in*/ const char *name)
{
    struct lfnde e;
    if (cchdir_findentry(dir, name, &e)) {
        vfat_errno = EALREADYEXISTS;
        return false;
    }

    return true;
}

static void cchdir_read_entries(struct cchdir *dir, u8 *buf)
{
    u32 offset = 0;
    u8 entry_type;
    struct lfnde e;
    u32 i;

    for (i = 0; i < dir->capacity; ++i) {
        entry_type = buf[offset];
        if (entry_type == NO_DIR_ENTRY) {
            break;
        }

        lfnde_readbuf(buf + offset, &e);
        alist_add(dir->entries, &e);
        offset += lfnde_count(&e) * FAT_DIR_ENTRY_SIZE;
    }
}

static u32 cchdir_write_entries(struct cchdir* dir, u8 *buf, u32 bufsize)
{
    u32 offset = 0;
    u32 i;

    struct lfnde e;
    for (i = 0; i < alist_count(dir->entries); ++i) {
        alist_get(dir->entries, i, &e);
        lfnde_writebuf(&e, buf + offset);
        offset += lfnde_count(&e) * FAT_DIR_ENTRY_SIZE;
    }

    if (offset < bufsize) {
        // Write the end-of-list marker.
        buf[offset] = NO_DIR_ENTRY;
        offset += 1;
    }

    // offset is equal to number of the actually written bytes.
    return offset;
}

void cchdir_formatdev(/*in*/ struct fdisk *dev,
                      /*in*/ u64 vol_size,
                      /*in*/ u16 bytes_per_sect,
                      /*in*/ u16 sect_per_clus)
{
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    vbr_create(&br, vol_size, bytes_per_sect, sect_per_clus);
    vbr_write(&br, dev);

    fat_create(&br, &fat);
    cchdir_createroot(&fat, &root);

    cchdir_write(&root, dev);
    fat_write(&fat, dev);

    cchdir_destruct(&root);
    fat_destruct(&fat);
}

void cchdir_readdir(/*in*/ struct fdisk *disk,
                    /*in*/ struct fat *fat,
                    /*in*/ u32 first_cluster,
                    /*in*/ bool root,
                    /*out*/ struct cchdir* dir)
{
    struct cch *cc = malloc(sizeof(struct cch));
    cc->fat = fat;
    cc->start_cluster = first_cluster;

    u64 size = cch_getsize(cc);
    u8 buf[size];
    cch_readdata(disk, cc, 0, size, buf);

    dir->chain = cc;
    dir->root = root;
    dir->capacity = size / FAT_DIR_ENTRY_SIZE;
    dir->entries = malloc(sizeof(struct alist));
    alist_create(dir->entries, sizeof(struct lfnde));

    cchdir_read_entries(dir, buf);
}

void cchdir_write(struct cchdir* dir, struct fdisk *disk)
{
    u32 nbytes = dir->capacity * FAT_DIR_ENTRY_SIZE;
    u8 buf[nbytes];
    u32 realbytes = cchdir_write_entries(dir, buf, nbytes);
    cch_writedata(disk, dir->chain, 0, realbytes, buf);
}

void cchdir_create(struct cch *cc, struct cchdir *dir)
{
    dir->chain = cc;
    dir->root = false;
    dir->capacity = cch_getsize(cc) / FAT_DIR_ENTRY_SIZE;
    dir->entries = malloc(sizeof(struct alist));
    alist_create(dir->entries, sizeof(struct lfnde));
}

void cchdir_createroot(struct fat *fat, struct cchdir *dir)
{
    struct vbr *br = fat->vbr;
    struct cch *cc = malloc(sizeof(struct cch));
    cch_create(cc, fat, 1);
    br->rootdir_first_cluster = cc->start_cluster;

    dir->chain = cc;
    dir->root = true;
    dir->capacity = cch_getsize(cc) / FAT_DIR_ENTRY_SIZE;
    dir->entries = malloc(sizeof(struct alist));
    alist_create(dir->entries, sizeof(struct lfnde));
}

void cchdir_readroot(/*in*/ struct fdisk *disk, /*in*/ struct fat *fat, /*out*/ struct cchdir *dir)
{
    cchdir_readdir(disk, fat, fat->vbr->rootdir_first_cluster, true, dir);
}

void cchdir_changesize(struct cchdir *dir, u32 fat32_entry_cnt)
{
    u32 size = fat32_entry_cnt * FAT_DIR_ENTRY_SIZE;
    u32 new_size = cch_setsize(dir->chain, size);
    dir->capacity = new_size / FAT_DIR_ENTRY_SIZE;
}

static u32 get_fat32_entry_cnt(struct cchdir *dir)
{
    u32 i;
    u32 n = 0;
    struct lfnde e;

    for (i = 0; i < alist_count(dir->entries); ++i) {
        alist_get(dir->entries, i, &e);
        n += 1 + e.fde->secondary_count;
    }

    return n;
}

void cchdir_addentry(struct cchdir *dir, struct lfnde *e)
{
    u32 new_cnt = get_fat32_entry_cnt(dir);
    new_cnt += 1 + e->fde->secondary_count;

    if (new_cnt > dir->capacity) {
        cchdir_changesize(dir, new_cnt);
    }

    alist_add(dir->entries, e);
}

void cchdir_getentry(/*in*/ struct cchdir *dir, /*in*/ u32 idx, /*out*/ struct lfnde *e)
{
    alist_get(dir->entries, idx, e);
}

bool cchdir_findentry(/*in*/ struct cchdir *dir, /*out*/ const char *name, /*out*/ struct lfnde *e)
{
    u32 i;
    char namebuf[256];

    for (i = 0; i < alist_count(dir->entries); ++i) {
        alist_get(dir->entries, i, e);
        lfnde_getname(e, namebuf);
        if (strcmp(name, namebuf) == 0) {
            return true;
        }
    }

    /* not found */
    return false;
}

bool cchdir_findentryidx(/*in*/ struct cchdir *dir, /*out*/ const char *name, /*out*/ u32 *idx)
{
    u32 i;
    char namebuf[256];
    struct lfnde e;

    for (i = 0; i < alist_count(dir->entries); ++i) {
        alist_get(dir->entries, i, &e);
        lfnde_getname(&e, namebuf);
        if (strcmp(name, namebuf) == 0) {
            *idx = i;
            return true;
        }
    }

    /* not found */
    return false;
}

void cchdir_removeentry(struct cchdir *dir, u32 idx)
{
    u32 new_cnt;

    alist_remove(dir->entries, idx);
    new_cnt = get_fat32_entry_cnt(dir);
    if (new_cnt > 0) {
        cchdir_changesize(dir, new_cnt);
    } else {
        cchdir_changesize(dir, 1); // Empty directory consists of 1 cluster
    }
}

bool cchdir_createsubdir(/*in*/ struct cchdir *parentdir, /*out*/ struct cchdir *subdir, /*out*/ struct lfnde* subde)
{
    struct lfnde dot;
    struct lfnde dotdot;
    struct fat *fat = parentdir->chain->fat;

    struct cch *cc = malloc(sizeof(struct cch));
    if (!cch_create(cc, fat, 1)) {
        return false;
    }

    lfnde_create(subde);
    lfnde_setisdir(subde, true);
    lfnde_setstartcluster(subde, cc->start_cluster);

    cchdir_create(cc, subdir);

    /* Add "." entry */
    lfnde_create(&dot);
    lfnde_setname(&dot, ".");
    lfnde_setisdir(&dot, true);
    lfnde_setstartcluster(&dot, subdir->chain->start_cluster);
    // TODO: copy date/time fields from entry to dot;
    cchdir_addentry(subdir, &dot);

    /* Add ".." entry */
    lfnde_create(&dotdot);
    lfnde_setname(&dotdot, "..");
    lfnde_setisdir(&dotdot, true);
    lfnde_setstartcluster(&dotdot, parentdir->chain->start_cluster);
    // TODO: copy date/time fields from entry to dotdot;
    cchdir_addentry(subdir, &dotdot);

    //cchdir_write(subdir, disk);
    return true;
}

bool cchdir_adddir(/*in*/ struct cchdir *dir, /*in*/ const char *name, /*out*/ struct lfnde *e)
{
    if (!check_unique_name(dir, name)) {
        return false;
    }

    struct cchdir subdir;
    if (!cchdir_createsubdir(dir, &subdir, e)) {
        return false;
    }

    lfnde_setname(e, name);
    cchdir_addentry(dir, e);

    return true;
}

bool cchdir_removedir(/*in*/ struct cchdir *dir, /*in*/ const char *name)
{
    struct lfnde e;
    struct cch cc;
    u32 idx;

    if (!cchdir_findentryidx(dir, name, &idx)) {
        return false;
    }

    cchdir_getentry(dir, idx, &e);

    cc.fat = dir->chain->fat;
    cc.start_cluster = e.sede->first_cluster;
    if (!cch_setlen(&cc, 0)) {
        return false;
    }

    cchdir_removeentry(dir, idx);
    return true;
}

bool cchdir_addfile(/*in*/ struct cchdir *dir, /*in*/ const char *name, /*out*/ struct lfnde *e)
{
    if (!check_unique_name(dir, name)) {
        return false;
    }

    lfnde_create(e);
    lfnde_setname(e, name);
    lfnde_setisdir(e, false);
    lfnde_setstartcluster(e, 0);
    lfnde_setdatalen(e, 0);
    cchdir_addentry(dir, e);

    return true;
}

void cchdir_getfile(/*in*/ struct fdisk *dev,
                    /*in*/ struct cchdir *dir,
                    /*in*/ struct lfnde *e,
                    /*out*/ struct cchfile *file)
{
    struct cch *cc = malloc(sizeof(struct cch));
    cc->fat = dir->chain->fat;
    cc->start_cluster = lfnde_getstartcluster(e);

    file->chain = cc;
    file->entry = e;
}

bool cchdir_getdir(/*in*/ struct fdisk *dev,
                   /*in*/ struct fat *fat,
                   /*in*/ struct lfnde *e,
                   /*out*/ struct cchdir *dir)
{
    u32 first_cluster = lfnde_getstartcluster(e);
    cchdir_readdir(dev, fat, first_cluster, false, dir);

    return true;
}

bool cchdir_move(/*in*/ struct fdisk *dev,
                 /*in*/ struct cchdir *src,
                 /*in*/ struct lfnde *e,
                 /*in*/ struct cchdir *dst,
                 /*in*/ const char *new_name)
{
    if (!check_unique_name(dst, new_name)) {
        return false;
    }

    u32 idx;
    char old_name[256];
    lfnde_getname(e, old_name);
    cchdir_findentryidx(src, old_name, &idx);

    cchdir_removeentry(src, idx);
    lfnde_setname(e, new_name);
    cchdir_addentry(dst, e);

    if (lfnde_isdir(e)) {
        struct cchdir subdir;
        cchdir_getdir(dev, src->chain->fat, e, &subdir);

        struct lfnde dotdot;
        cchdir_findentry(&subdir, "..", &dotdot);
        assert(lfnde_getstartcluster(&dotdot) == src->chain->start_cluster);
        lfnde_setstartcluster(&dotdot, dst->chain->start_cluster);

        // Write subdir to the disk
        cchdir_write(&subdir, dev);

        cchdir_destruct(&subdir);
    }

    return true;
}

bool cchdir_setname(/*in*/ struct fdisk *dev,
                    /*in*/ struct cchdir *dir,
                    /*in*/ struct lfnde *e,
                    /*in*/ const char *name)
{
    return cchdir_move(dev, dir, e, dir, name);
}

void cchdir_destruct(/*in*/ struct cchdir *dir)
{
    struct lfnde e;
    u32 i;

    for (i = 0; i < alist_count(dir->entries); ++i) {
        alist_get(dir->entries, i, &e);
        lfnde_destruct(&e);
    }

    alist_destruct(dir->entries);
    free(dir->entries);
    free(dir->chain);
}
