#include <assert.h>

#include "../include/alist.h"
#include "../include/fat.h"
#include "../include/cchdir.h"
#include "../include/cchfile.h"
#include "../include/lfnde.h"
#include "../include/BootSector.h"

int32_t __vfat_errno;

using namespace org::vfat;

static bool check_unique_name(/*in*/ struct cchdir *dir, /*in*/ const char *name)
{
    struct lfnde e;
    if (cchdir_findentry(dir, name, &e)) {
        __vfat_errno = EALREADYEXISTS;
        return false;
    }

    return true;
}

static void cchdir_read_entries(struct cchdir *dir, uint8_t *buffer)
{
    uint32_t offset = 0;
    uint8_t entry_type;
    struct lfnde e;
    uint32_t i;

    for (i = 0; i < dir->capacity; ++i) {
        entry_type = buffer[offset];
        if (entry_type == NO_DIR_ENTRY) {
            break;
        }

        lfnde_readbuf(buffer + offset, &e);
        alist_add(dir->entries, &e);
        offset += lfnde_count(&e) * FAT_DIR_ENTRY_SIZE;
    }
}

static uint32_t cchdir_write_entries(struct cchdir* dir, uint8_t *buffer, uint32_t bufsize)
{
    uint32_t offset = 0;
    uint32_t i;

    struct lfnde e;
    for (i = 0; i < alist_count(dir->entries); ++i) {
        alist_get(dir->entries, i, &e);
        lfnde_writebuf(&e, buffer + offset);
        offset += lfnde_count(&e) * FAT_DIR_ENTRY_SIZE;
    }

    if (offset < bufsize) {
        // Write the end-of-list marker.
        buffer[offset] = NO_DIR_ENTRY;
        offset += 1;
    }

    // offset is equal to number of the actually written bytes.
    return offset;
}

void cchdir_formatdev(/*in*/ org::vfat::FileDisk *device,
                      /*in*/ uint64_t vol_size,
                      /*in*/ uint16_t bytes_per_sect,
                      /*in*/ uint16_t sect_per_clus)
{
    org::vfat::BootSector bootSector;
    Fat fat(&bootSector);
    struct cchdir root;

    bootSector.Create(vol_size, bytes_per_sect, sect_per_clus);
    bootSector.Write(device);

    fat.Create();
    cchdir_createroot(&fat, &root);

    cchdir_write(&root, device);
    fat.Write(device);

    cchdir_destruct(&root);
    //fat_destruct(&fat); will be invoked automatically
}

void cchdir_readdir(/*in*/ org::vfat::FileDisk *device,
                    /*in*/ Fat *fat,
                    /*in*/ uint32_t first_cluster,
                    /*in*/ bool root,
                    /*out*/ struct cchdir* dir)
{
    struct cch *cc = static_cast<struct cch *>(malloc(sizeof(struct cch)));
    cc->fat = fat;
    cc->start_cluster = first_cluster;

    uint64_t size = cch_getsize(cc);
    uint8_t buffer[size];
    cch_readdata(device, cc, 0, size, buffer);

    dir->chain = cc;
    dir->root = root;
    dir->capacity = size / FAT_DIR_ENTRY_SIZE;
    dir->entries = static_cast<struct alist *>(malloc(sizeof(struct alist)));
    alist_create(dir->entries, sizeof(struct lfnde));

    cchdir_read_entries(dir, buffer);
}

void cchdir_write(struct cchdir* dir, org::vfat::FileDisk *device)
{
    uint32_t nbytes = dir->capacity * FAT_DIR_ENTRY_SIZE;
    uint8_t buf[nbytes];
    uint32_t realbytes = cchdir_write_entries(dir, buf, nbytes);
    cch_writedata(device, dir->chain, 0, realbytes, buf);
}

void cchdir_create(struct cch *cc, struct cchdir *dir)
{
    dir->chain = cc;
    dir->root = false;
    dir->capacity = cch_getsize(cc) / FAT_DIR_ENTRY_SIZE;
    dir->entries = static_cast<struct alist *>(malloc(sizeof(struct alist)));
    alist_create(dir->entries, sizeof(struct lfnde));
}

void cchdir_createroot(Fat *fat, struct cchdir *dir)
{
    BootSector *bootSector = fat->GetBootSector();
    struct cch *cc = static_cast<struct cch *>(malloc(sizeof(struct cch)));
    cch_create(cc, fat, 1);
    bootSector->SetRootDirFirstCluster(cc->start_cluster);

    dir->chain = cc;
    dir->root = true;
    dir->capacity = cch_getsize(cc) / FAT_DIR_ENTRY_SIZE;
    dir->entries = static_cast<struct alist *>(malloc(sizeof(struct alist)));
    alist_create(dir->entries, sizeof(struct lfnde));
}

void cchdir_readroot(/*in*/ org::vfat::FileDisk *device, /*in*/ Fat *fat, /*out*/ struct cchdir *dir)
{
    cchdir_readdir(device, fat, fat->GetBootSector()->GetRootDirFirstCluster(), true, dir);
}

void cchdir_changesize(struct cchdir *dir, uint32_t fat32_entry_cnt)
{
    uint32_t size = fat32_entry_cnt * FAT_DIR_ENTRY_SIZE;
    uint32_t new_size = cch_setsize(dir->chain, size);
    dir->capacity = new_size / FAT_DIR_ENTRY_SIZE;
}

static uint32_t get_fat32_entry_cnt(struct cchdir *dir)
{
    uint32_t i;
    uint32_t n = 0;
    struct lfnde e;

    for (i = 0; i < alist_count(dir->entries); ++i) {
        alist_get(dir->entries, i, &e);
        n += 1 + e.fde->secondary_count;
    }

    return n;
}

void cchdir_addentry(struct cchdir *dir, struct lfnde *e)
{
    uint32_t new_cnt = get_fat32_entry_cnt(dir);
    new_cnt += 1 + e->fde->secondary_count;

    if (new_cnt > dir->capacity) {
        cchdir_changesize(dir, new_cnt);
    }

    alist_add(dir->entries, e);
}

void cchdir_getentry(/*in*/ struct cchdir *dir, /*in*/ uint32_t idx, /*out*/ struct lfnde *e)
{
    alist_get(dir->entries, idx, e);
}

bool cchdir_findentry(/*in*/ struct cchdir *dir, /*in*/ const char *name, /*out*/ struct lfnde *e)
{
    uint32_t i;
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

bool cchdir_findentryidx(/*in*/ struct cchdir *dir, /*in*/ const char *name, /*out*/ uint32_t *idx)
{
    uint32_t i;
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

void cchdir_removeentry(struct cchdir *dir, uint32_t idx)
{
    uint32_t new_cnt;

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
    Fat *fat = parentdir->chain->fat;

    struct cch *cc = static_cast<struct cch *>(malloc(sizeof(struct cch)));
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

bool cchdir_adddir(/*in*/ struct cchdir *dir,
                   /*in*/ const char *name,
                   /*out*/ struct lfnde *subde,
                   /*out*/ struct cchdir *subdir)
{
    if (!check_unique_name(dir, name)) {
        return false;
    }

    if (!cchdir_createsubdir(dir, subdir, subde)) {
        return false;
    }

    lfnde_setname(subde, name);
    cchdir_addentry(dir, subde);

    return true;
}

bool cchdir_removedir(/*in*/ struct cchdir *dir, /*in*/ const char *name)
{
    struct lfnde e;
    struct cch cc;
    uint32_t idx;

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

void cchdir_getfile(/*in*/ struct cchdir *dir,
                    /*in*/ struct lfnde *e,
                    /*out*/ struct cchfile *file)
{
    struct cch *cc = static_cast<struct cch *>(malloc(sizeof(struct cch)));
    cc->fat = dir->chain->fat;
    cc->start_cluster = lfnde_getstartcluster(e);

    file->chain = cc;
    file->entry = e;
}

bool cchdir_getdir(/*in*/ org::vfat::FileDisk *device,
                   /*in*/ Fat *fat,
                   /*in*/ struct lfnde *e,
                   /*out*/ struct cchdir *dir)
{
    uint32_t first_cluster = lfnde_getstartcluster(e);
    cchdir_readdir(device, fat, first_cluster, false, dir);

    return true;
}

bool cchdir_move(/*in*/ org::vfat::FileDisk *device,
                 /*in*/ struct cchdir *src,
                 /*in*/ struct lfnde *e,
                 /*in*/ struct cchdir *dst,
                 /*in*/ const char *new_name)
{
    if (!check_unique_name(dst, new_name)) {
        return false;
    }

    uint32_t idx;
    char old_name[256];
    lfnde_getname(e, old_name);
    cchdir_findentryidx(src, old_name, &idx);
    cchdir_removeentry(src, idx);
    lfnde_setname(e, new_name);
    cchdir_addentry(dst, e);

    if (lfnde_isdir(e)) {
        struct cchdir dir;
        cchdir_getdir(device, src->chain->fat, e, &dir);

        struct lfnde dotdot;
        cchdir_findentry(&dir, "..", &dotdot);
        assert(lfnde_getstartcluster(&dotdot) == src->chain->start_cluster);
        lfnde_setstartcluster(&dotdot, dst->chain->start_cluster);

        // Write dir to the disk
        cchdir_write(&dir, device);
        cchdir_destruct(&dir);
    }

    return true;
}

bool cchdir_copyfile(/*in*/ org::vfat::FileDisk *device,
                     /*in*/ struct cchdir *src,
                     /*in*/ struct lfnde *e,
                     /*in*/ struct cchdir *dst)
{
    char namebuf[256];
    lfnde_getname(e, namebuf);

    struct lfnde copye;
    if (!cchdir_addfile(dst, namebuf, &copye)) {
        return false;
    }

    // Copy the file content
    const int nbytes = 4096;
    uint8_t buf[nbytes];

    struct cchfile orig;
    cchdir_getfile(src, e, &orig);

    struct cchfile copy;
    cchdir_getfile(src, &copye, &copy);

    uint32_t pos = 0;
    uint32_t nread;
    cchfile_read(device, &orig, pos, nbytes, &nread, buf);
    while (nread > 0) {
        cchfile_write(device, &copy, pos, nread, buf);
        pos += nread;
        cchfile_read(device, &orig, pos, nbytes, &nread, buf);
    }

    // Free memory
    cchfile_destruct(&orig);
    cchfile_destruct(&copy);

    return true;
}

bool cchdir_copydir(/*in*/ org::vfat::FileDisk *device,
                    /*in*/ struct cchdir *src,
                    /*in*/ struct lfnde *e,
                    /*in*/ struct cchdir *dst)
{
    Fat *fat = src->chain->fat;

    char namebuf[256];
    lfnde_getname(e, namebuf);

    struct cchdir orig;
    if (!cchdir_getdir(device, fat, e, &orig)) {
        return false;
    }

    struct lfnde copye;
    struct cchdir copy;
    if (!cchdir_adddir(dst, namebuf, &copye, &copy)) {
        return false;
    }

    struct lfnde child;
    uint32_t i;
    for (i = 0; i < alist_count(orig.entries); ++i) {
        cchdir_getentry(&orig, i, &child);
        if (lfnde_isdir(&child)) {
            lfnde_getname(&child, namebuf);
            if (strcmp(namebuf, ".") == 0 || strcmp(namebuf, "..") == 0) {
                continue;
            }

            if (!cchdir_copydir(device, &orig, &child, &copy)) {
                return false;
            }
        } else {
            if (!cchdir_copyfile(device, &orig, &child, &copy)) {
                return false;
            }
        }
    }

    cchdir_write(&copy, device);

    cchdir_destruct(&orig);
    cchdir_destruct(&copy);

    return true;
}

bool cchdir_setname(/*in*/ org::vfat::FileDisk *device,
                    /*in*/ struct cchdir *dir,
                    /*in*/ struct lfnde *e,
                    /*in*/ const char *name)
{
    return cchdir_move(device, dir, e, dir, name);
}

void cchdir_destruct(/*in*/ struct cchdir *dir)
{
    struct lfnde e;
    uint32_t i;

    for (i = 0; i < alist_count(dir->entries); ++i) {
        alist_get(dir->entries, i, &e);
        lfnde_destruct(&e);
    }

    alist_destruct(dir->entries);
    free(dir->entries);
    free(dir->chain);
}
