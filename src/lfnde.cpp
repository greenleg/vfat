#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../include/binaryreader.h"
#include "../include/alist.h"
#include "../include/lfnde.h"

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

/*
 * Attribute Offset Size Mask
 * --------------------------
 * Reserved2 6      10
 * Archive   5      1    0x20
 * Directory 4      1    0x10
 * Reserved1 3      1
 * System    2      1    0x04
 * Hidden    1      1    0x02
 * Read-Only 0      1    0x01
 */
#define DIRECTORY_MASK 0x10

static void fde_readbuf(uint8_t *buf, struct fde *e)
{
    e->entry_type = read_u8(buf, FDE_ENTRYTYPE_OFFSET);
    e->secondary_count = read_u8(buf, FDE_SECONDARYCOUNT_OFFSET);
    e->attributes = read_u16(buf, FDE_ATTRIBUTES_OFFSET);
    e->create = read_u32(buf, FDE_CREATE_OFFSET);
    e->last_modified = read_u32(buf, FDE_LASTMODIFIED_OFFSET);
    e->last_accessed = read_u32(buf, FDE_LASTACCESSED_OFFSET);
}

static void fde_writebuf(struct fde *e, uint8_t *buf)
{
    write_u8(buf, FDE_ENTRYTYPE_OFFSET, e->entry_type);
    write_u8(buf, FDE_SECONDARYCOUNT_OFFSET, e->secondary_count);
    write_u16(buf, FDE_ATTRIBUTES_OFFSET, e->attributes);
    write_u32(buf, FDE_CREATE_OFFSET, e->create);
    write_u32(buf, FDE_LASTMODIFIED_OFFSET, e->last_modified);
    write_u32(buf, FDE_LASTACCESSED_OFFSET, e->last_accessed);
}

static void sede_readbuf(uint8_t *buf, struct sede *e)
{
    e->entry_type = read_u8(buf, SEDE_ENTRYTYPE_OFFSET);
    e->secondary_flags = read_u8(buf, SEDE_SECONDARYFLAGS_OFFSET);
    e->name_length = read_u8(buf, SEDE_NAMELENGTH_OFFSET);
    e->first_cluster = read_u32(buf, SEDE_FIRSTCLUSTER_OFFSET);
    e->data_length = read_u64(buf, SEDE_DATALENGTH_OFFSET);
}

static void sede_writebuf(struct sede *e, uint8_t *buf)
{
    write_u8(buf, SEDE_ENTRYTYPE_OFFSET, e->entry_type);
    write_u8(buf, SEDE_SECONDARYFLAGS_OFFSET, e->secondary_flags);
    write_u8(buf, SEDE_NAMELENGTH_OFFSET, e->name_length);
    write_u32(buf, SEDE_FIRSTCLUSTER_OFFSET, e->first_cluster);
    write_u64(buf, SEDE_DATALENGTH_OFFSET, e->data_length);
}

static void fnede_readbuf(uint8_t *buf, struct fnede *e)
{
    e->entry_type = read_u8(buf, FNEDE_ENTRYTYPE_OFFSET);
    e->secondary_flags = read_u8(buf, FNEDE_SECODARYFLAGS_OFFSET);
    memcpy(e->name, buf + FNEDE_FILENAME_OFFSET, FNEDE_UNAME_LENGTH);
}

static void fnede_writebuf(struct fnede *e, uint8_t *buf)
{
    write_u8(buf, FNEDE_ENTRYTYPE_OFFSET, e->entry_type);
    write_u8(buf, FNEDE_SECODARYFLAGS_OFFSET, e->secondary_flags);
    memcpy(buf + FNEDE_FILENAME_OFFSET, e->name, FNEDE_UNAME_LENGTH);
}

void lfnde_destruct(struct lfnde *e)
{
    free(e->fde);
    free(e->sede);
    alist_destruct(e->fnede_list);
    free(e->fnede_list);
}

uint16_t lfnde_count(struct lfnde *e)
{
    return 1 + e->fde->secondary_count;
}

void lfnde_create(struct lfnde *e)
{
    e->fde = static_cast<struct fde *>(malloc(sizeof(struct fde)));
    e->sede = static_cast<struct sede *>(malloc(sizeof(struct sede)));
    e->fnede_list = static_cast<struct alist *>(malloc(sizeof(struct alist)));
    alist_create(e->fnede_list, sizeof(struct fnede));

    // Set valid default values.
    e->fde->entry_type = FILE_DIR_ENTRY;
    e->fde->secondary_count = 1;
    e->sede->entry_type = STREAMEXT_DIR_ENTRY;
    e->sede->name_length = 0;
    e->sede->first_cluster = 0;
}

void lfnde_readbuf(uint8_t *buf, struct lfnde *e)
{
    assert(buf[0] == FILE_DIR_ENTRY);
    e->fde = static_cast<struct fde *>(malloc(sizeof(struct fde)));
    fde_readbuf(buf, e->fde);
    buf += FAT_DIR_ENTRY_SIZE;

    assert(buf[0] == STREAMEXT_DIR_ENTRY);
    e->sede = static_cast<struct sede *>(malloc(sizeof(struct sede)));
    sede_readbuf(buf, e->sede);
    buf += FAT_DIR_ENTRY_SIZE;

    struct fnede fnede;
    uint8_t i;

    e->fnede_list = static_cast<struct alist *>(malloc(sizeof(struct alist)));
    alist_create(e->fnede_list, sizeof(struct fnede));

    for (i = 0; i < e->fde->secondary_count - 1; ++i) {
        assert(buf[0] == FILENAMEEXT_DIR_ENTRY);
        fnede_readbuf(buf, &fnede);
        alist_add(e->fnede_list, &fnede);
        buf += FAT_DIR_ENTRY_SIZE;
    }
}

void lfnde_writebuf(struct lfnde *e, uint8_t *buf)
{
    fde_writebuf(e->fde, buf);
    buf += FAT_DIR_ENTRY_SIZE;

    sede_writebuf(e->sede, buf);
    buf += FAT_DIR_ENTRY_SIZE;

    struct fnede fnede;
    uint8_t i;
    for (i = 0; i < e->fde->secondary_count - 1; ++i) {        
        alist_get(e->fnede_list, i, &fnede);
        fnede_writebuf(&fnede, buf);
        buf += FAT_DIR_ENTRY_SIZE;
    }
}

bool lfnde_isdir(/*in*/ struct lfnde *e)
{
    uint16_t attr = e->fde->attributes;
    return (attr & DIRECTORY_MASK) != 0;
}

bool lfnde_isfile(/*in*/ struct lfnde *e)
{
    uint16_t attr = e->fde->attributes;
    return (attr & DIRECTORY_MASK) == 0;
}

void lfnde_setisdir(/*in*/ struct lfnde *e, /*in*/ bool val)
{
    uint16_t attr = e->fde->attributes;
    if (val) {
        attr |= DIRECTORY_MASK;
    } else {
        attr &= ~DIRECTORY_MASK;
    }

    e->fde->attributes = attr;
}

uint64_t lfnde_getdatalen(/*in*/ struct lfnde *e)
{
    return e->sede->data_length;
}

void lfnde_setdatalen(/*in*/ struct lfnde *e, /*in*/ uint64_t len)
{
    e->sede->data_length = len;
}

uint32_t lfnde_getstartcluster(/*in*/ struct lfnde *e)
{
    return e->sede->first_cluster;
}

void lfnde_setstartcluster(/*in*/ struct lfnde *e, /*in*/ uint32_t start_cluster)
{
    e->sede->first_cluster = start_cluster;
}

void lfnde_getname(/*in*/ struct lfnde *e, /*out*/ char *name)
{
    uint8_t len = e->sede->name_length;
    uint8_t char_idx;
    uint8_t fnede_cnt = (len + FNEDE_UNAME_LENGTH - 1) / FNEDE_UNAME_LENGTH;
    uint8_t fnede_idx;
    struct fnede fnede;
    uint8_t i;

    char_idx = 0;
    for (fnede_idx = 0; fnede_idx < fnede_cnt - 1; ++fnede_idx) {
        alist_get(e->fnede_list, fnede_idx, &fnede);
        for (i = 0; i < FNEDE_UNAME_LENGTH; ++i) {
            name[char_idx + i] = fnede.name[i];
        }

        char_idx += FNEDE_UNAME_LENGTH;
    }

    alist_get(e->fnede_list, fnede_idx, &fnede);
    for (i = 0; i < len - char_idx; ++i) {
        name[char_idx + i] = fnede.name[i];
    }

    name[len] = '\0';
}

void lfnde_setname(/*in*/ struct lfnde *e, /*in*/ const char *name)
{
    struct fnede fnede;
    uint8_t len = strlen(name);
    uint8_t fnede_cnt = alist_count(e->fnede_list);
    uint8_t new_fnede_cnt = (len + (FNEDE_UNAME_LENGTH - 1)) / FNEDE_UNAME_LENGTH;
    uint8_t i, char_idx, fnede_idx;

    // Clear list
    for (i = 0; i < fnede_cnt; ++i) {
        alist_remove(e->fnede_list, 0);
    }

    fnede.entry_type = FILENAMEEXT_DIR_ENTRY;

    // Fill list with the new values
    char_idx = 0;
    for (fnede_idx = 0; fnede_idx < new_fnede_cnt - 1; ++fnede_idx) {
        for (i = 0; i < FNEDE_UNAME_LENGTH; ++i) {
            fnede.name[i] = name[char_idx + i];
        }

        char_idx += FNEDE_UNAME_LENGTH;
        alist_add(e->fnede_list, &fnede);
    }

    // Special case for the last item
    for (i = 0; i < len - char_idx; ++i) {
        fnede.name[i] = name[char_idx + i];
    }

    alist_add(e->fnede_list, &fnede);
    e->sede->name_length = len;
    e->fde->secondary_count = new_fnede_cnt + 1;
}
