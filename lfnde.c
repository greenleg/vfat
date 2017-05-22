#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "binaryreader.h"
#include "alist.h"
#include "lfnde.h"

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

void lfnde_free(struct lfnde *e)
{
    free(e->fde);
    free(e->sede);
    alist_free(e->fnede_list);
    free(e->fnede_list);
}

u16 lfnde_count(struct lfnde *e)
{
    return 1 + e->fde->secondary_count;
}

void lfnde_readbuf(u8 *buf, struct lfnde *e)
{
    assert(buf[0] == FILE_DIR_ENTRY);
    e->fde = malloc(sizeof(struct fde));
    fde_readbuf(buf, e->fde);
    buf += FAT_DIR_ENTRY_SIZE;

    assert(buf[0] == STREAMEXT_DIR_ENTRY);
    e->sede = malloc(sizeof(struct sede));
    sede_readbuf(buf, e->sede);
    buf += FAT_DIR_ENTRY_SIZE;

    struct fnede fnede;
    u8 i;

    e->fnede_list = malloc(sizeof(struct alist));
    alist_create(e->fnede_list, sizeof(struct fnede));

    for (i = 0; i < e->fde->secondary_count - 1; ++i) {
        assert(buf[0] == FILENAMEEXT_DIR_ENTRY);
        fnede_readbuf(buf, &fnede);
        alist_add(e->fnede_list, &fnede);
        buf += FAT_DIR_ENTRY_SIZE;
    }
}

void lfnde_writebuf(struct lfnde *e, u8 *buf)
{
    fde_writebuf(e->fde, buf);
    buf += FAT_DIR_ENTRY_SIZE;

    sede_writebuf(e->sede, buf);
    buf += FAT_DIR_ENTRY_SIZE;

    u8 i;
    for (i = 0; i < e->fde->secondary_count - 1; ++i) {
        assert(buf[0] == FILENAMEEXT_DIR_ENTRY);
        fnede_writebuf(alist_get(e->fnede_list, i), buf);
        buf += FAT_DIR_ENTRY_SIZE;
    }
}
