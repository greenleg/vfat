#ifndef VFAT_FILE_DIRECTORY_ENTRY_H
#define VFAT_FILE_DIRECTORY_ENTRY_H

#include <stdint.h>

#include "common.h"
#include "fdisk.h"

#define FAT_DIR_ENTRY_SIZE 32

#define FILE_DIR_ENTRY 0x85
#define STREAMEXT_DIR_ENTRY 0xC0
#define FILENAMEEXT_DIR_ENTRY 0xC1
#define NO_DIR_ENTRY 0x00

#define FNEDE_FILENAME_LENGTH 30

struct fde
{
    u8 entry_type;
    u8 secondary_count;
    u16 attributes;
    u32 create;
    u32 last_modified;
    u32 last_accessed;
};

struct sede
{
    u8 entry_type;
    u8 secondary_flags;
    u8 name_length;
    u32 first_cluster;
    u64 data_length;
};

struct fnede
{
    u8 entry_type;
    u8 secondary_flags;
    u8 name[FNEDE_FILENAME_LENGTH];
};

struct lfnde
{
    struct fde *fde;
    struct sede *sede;
    struct fnede **fnede_list;
};

void lfnde_readbuf(u8 *buf, struct lfnde *e);
void lfnde_writebuf(struct lfnde *e, u8 *buf);
u32  lfnde_getlen(struct lfnde *e);
void lfnde_getname(struct lfnde *e, char *name);
void lfnde_setname(struct lfnde *e, const char *name);
u8 lfnde_isfile(struct lfnde *e);
/*u8 lfnde_getentrycnt(struct lfnde *e);*/


#endif /* VFAT_FILE_DIRECTORY_ENTRY_H */
