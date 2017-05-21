#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "binaryreader.h"
#include "filedirentry.h"

#define FDE_ENTRYTYPE_OFFSET 0
#define FDE_SECONDARYCOUNT_OFFSET 1
#define FDE_ATTRIBUTES_OFFSET 4
#define FDE_CREATE_OFFSET 8
#define FDE_LASTMODIFIED_OFFSET 12
#define FDE_LASTACCESSED_OFFSET 16

#define SEDE_ENTRYTYPE_OFFSET 0
#define SEDE_SECONDARYFLAGS_OFFSET 1
#define SEDE_NAMELENGTH_OFFSET 3
#define SEDE_FIRSTCLUSTER_OFFSET 20
#define SEDE_DATALENGTH_OFFSET 24

#define FNEDE_ENTRYTYPE_OFFSET 0
#define FNEDE_SECODARYFLAGS_OFFSET 1
#define FNEDE_FILENAME_OFFSET 2

void fde_readbuf(u8 *buf, struct fde *e)
{
    e->entry_type = read_u8(buf, FDE_ENTRYTYPE_OFFSET);
    e->secondary_count = read_u8(buf, FDE_SECONDARYCOUNT_OFFSET);
    e->attributes = read_u16(buf, FDE_ATTRIBUTES_OFFSET);
    e->create = read_u32(buf, FDE_CREATE_OFFSET);
    e->last_modified = read_u32(buf, FDE_LASTMODIFIED_OFFSET);
    e->last_accessed = read_u32(buf, FDE_LASTACCESSED_OFFSET);
}

void fde_writebuf(struct fde *e, u8 *buf)
{
    write_u8(buf, FDE_ENTRYTYPE_OFFSET, e->entry_type);
    write_u8(buf, FDE_SECONDARYCOUNT_OFFSET, e->secondary_count);
    write_u16(buf, FDE_ATTRIBUTES_OFFSET, e->attributes);
    write_u32(buf, FDE_CREATE_OFFSET, e->create);
    write_u32(buf, FDE_LASTMODIFIED_OFFSET, e->last_modified);
    write_u32(buf, FDE_LASTACCESSED_OFFSET, e->last_accessed);
}

void sede_readbuf(u8 *buf, struct sede *e)
{
    e->entry_type = read_u8(buf, SEDE_ENTRYTYPE_OFFSET);
    e->secondary_flags = read_u8(buf, SEDE_SECONDARYFLAGS_OFFSET);
    e->name_length = read_u8(buf, SEDE_NAMELENGTH_OFFSET);
    e->first_cluster = read_u32(buf, SEDE_FIRSTCLUSTER_OFFSET);
    e->data_length = read_u64(buf, SEDE_DATALENGTH_OFFSET);
}

void sede_writebuf(struct sede *e, u8 *buf)
{
    write_u8(buf, SEDE_ENTRYTYPE_OFFSET, e->entry_type);
    write_u8(buf, SEDE_SECONDARYFLAGS_OFFSET, e->secondary_flags);
    write_u8(buf, SEDE_NAMELENGTH_OFFSET, e->name_length);
    write_u32(buf, SEDE_FIRSTCLUSTER_OFFSET, e->first_cluster);
    write_u64(buf, SEDE_DATALENGTH_OFFSET, e->data_length);
}

void fnede_readbuf(u8 *buf, struct fnede *e)
{
    e->entry_type = read_u8(buf, FNEDE_ENTRYTYPE_OFFSET);
    e->secondary_flags = read_u8(buf, FNEDE_SECODARYFLAGS_OFFSET);
    memcpy(e->name, buf, FNEDE_FILENAME_OFFSET);
}

void fnede_writebuf(struct fnede *e, u8 *buf)
{
    write_u8(buf, FNEDE_ENTRYTYPE_OFFSET, e->entry_type);
    write_u8(buf, FNEDE_SECODARYFLAGS_OFFSET, e->secondary_flags);
    memcpy(buf, e->name, FNEDE_FILENAME_OFFSET);
}

void lfnde_readbuf(u8 *buf, struct lfnde *e)
{
    assert(buf[0] == FILE_DIR_ENTRY);
    e->fde = malloc(sizeof(struct fde));
    fde_readbuf(buf, e->fde);

    buf += FAT_DIR_ENTRY_SIZE;

    e->sede = malloc(sizeof(struct sede));
    assert(buf[0] == STREAMEXT_DIR_ENTRY);
    sede_readbuf(buf, e->sede);

    buf += FAT_DIR_ENTRY_SIZE;

    u32 fnede_cnt = e->fde->secondary_count - 1;
    u32 i;

    e->fnede_list = malloc(sizeof(struct fnede) * fnede_cnt);

    for (i = 0; i < fnede_cnt; ++i) {
        assert(buf[0] == FILENAMEEXT_DIR_ENTRY);
        e->fnede_list[i] = malloc(sizeof(struct fnede));
        fnede_readbuf(buf, e->fnede_list[i]);

        buf += FAT_DIR_ENTRY_SIZE;
    }
}
