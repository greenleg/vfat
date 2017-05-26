#include "alist.h"
#include "fat.h"
#include "cchdir.h"
#include "lfnde.h"

static void check_unique_name(/*in*/ const char *name)
{
    // TODO:
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

void cchdir_read(struct fdisk *disk, struct fat *fat, struct cchdir* dir, u32 first_cluster, bool root)
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

void cchdir_readroot(struct fdisk *disk, struct fat *fat, struct cchdir *dir)
{
    cchdir_read(disk, fat, dir, fat->vbr->rootdir_first_cluster, true);
}

void cchdir_changesize(struct cchdir *dir, u32 entry_cnt)
{
    u32 size = entry_cnt * FAT_DIR_ENTRY_SIZE;
    u32 new_size = cch_setsize(dir->chain, size);
    dir->capacity = new_size / FAT_DIR_ENTRY_SIZE;
}

void cchdir_addentry(struct cchdir *dir, struct lfnde *e)
{
    u32 new_count = alist_count(dir->entries) + (1 + e->fde->secondary_count);
    if (new_count > dir->capacity) {
        cchdir_changesize(dir, new_count);
    }

    alist_add(dir->entries, e);
}

void cchdir_getentry(struct cchdir *dir, u32 idx, struct lfnde *e)
{
    alist_get(dir->entries, idx, e);
}

int cchdir_findentry(/*in*/ struct cchdir *dir, /*out*/ const char *name, /*out*/ struct lfnde *e)
{
    u32 i;
    char namebuf[256];
    for (i = 0; i < alist_count(dir->entries); ++i) {
        alist_get(dir->entries, i, e);
        lfnde_getname(e, namebuf);
        if (strcmp(name, namebuf) == 0) {
            return 0;
        }
    }

    /* not found */
    return -1;
}

void cchdir_removeentry(struct cchdir *dir, u32 idx)
{
    alist_remove(dir->entries, idx);
    cchdir_changesize(dir, alist_count(dir->entries));
}

void cchdir_createsubdir(/*in*/ struct cchdir *parentdir, /*out*/ struct cchdir *subdir, /*out*/ struct lfnde* subde)
{
    struct lfnde dot;
    struct lfnde dotdot;
    struct fat *fat = parentdir->chain->fat;

    struct cch *cc = malloc(sizeof(struct cch));
    cch_create(cc, fat, 1);

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
}

int cchdir_adddir(/*in*/ struct cchdir *dir, /*in*/ const char *name, /*out*/ struct lfnde *e)
{
    check_unique_name(name);

    struct cchdir subdir;
    cchdir_createsubdir(dir, &subdir, e);
    lfnde_setname(e, name);

    cchdir_addentry(dir, e);

    return 0;
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
