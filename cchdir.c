#include "arraylist.h"
#include "fat.h"
#include "cchdir.h"
#include "filedirentry.h"

static void cchdir_read_entries(struct cchdir *dir, u8 *buf)
{
    u32 offset = 0;
    u8 entry_type;
    struct lfnde* e;
    u32 i;

    for (i = 0; i < dir->capacity; ++i) {
        entry_type = buf[offset];
        if (entry_type == NO_DIR_ENTRY) {
            break;
        }

        e = malloc(sizeof(struct lfnde));
        lfnde_readbuf(buf + offset, e);
        alist_add(dir->entries, e);
        offset += (1 + e->fde->secondary_count) * FAT_DIR_ENTRY_SIZE;
    }
}

void cchdir_write(struct cchdir* dir, struct fdisk *disk)
{

}

void cchdir_read(struct fdisk *disk, struct cchdir* dir)
{

}

void cchdir_createroot(struct fat *fat, struct cchdir *dir)
{
    struct vbr *br = fat->vbr;
    struct cch cc;
    cch_create(&cc, fat, 1);
    br->rootdir_first_cluster = cc.start_cluster;

    struct alist list;
    alist_create(&list, sizeof(struct lfnde *), 0);

    dir->chain = &cc;
    dir->is_root = 1;
    dir->capacity = cch_getsize(&cc) / FAT_DIR_ENTRY_SIZE;
    dir->entries = &list;
}

void cchdir_readroot(struct fdisk *disk, struct fat *fat, struct cchdir *dir)
{
    struct cch *cc = malloc(sizeof(struct cch));
    cc->fat = fat;
    cc->start_cluster = fat->vbr->rootdir_first_cluster;

    u64 size = cch_getsize(cc);
    u8 buf[size];
    cch_readdata(disk, cc, 0, size, buf);

    dir->chain = cc;
    dir->is_root = 1;
    dir->capacity = size / FAT_DIR_ENTRY_SIZE;
    alist_create(dir->entries, sizeof(struct lfnde *), dir->capacity);

    cchdir_read_entries(dir, buf);
}

void cchdir_changesize(struct cchdir *dir, u32 entry_cnt)
{
    u32 size = entry_cnt * FAT_DIR_ENTRY_SIZE;
    u32 new_size = cch_setsize(dir->chain, size);
    dir->capacity = new_size / FAT_DIR_ENTRY_SIZE;
}

void cchdir_add(struct cchdir *dir, struct lfnde *e)
{
    u32 new_entry_cnt = dir->entries->cnt + (1 + e->fde->secondary_count);
    if (new_entry_cnt > dir->capacity) {
        cchdir_changesize(dir, new_entry_cnt);
    }

    alist_add(dir->entries, e);
}
