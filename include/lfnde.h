#ifndef VFAT_LFNDE_H
#define VFAT_LFNDE_H

#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "FileDisk.h"

#define FAT_DIR_ENTRY_SIZE 32

#define FILE_DIR_ENTRY 0x85
#define STREAMEXT_DIR_ENTRY 0xC0
#define FILENAMEEXT_DIR_ENTRY 0xC1
#define NO_DIR_ENTRY 0x00

#define FNEDE_NAME_LENGTH 30
#define FNEDE_UNAME_LENGTH 15

struct fde
{
    uint8_t entry_type;
    uint8_t secondary_count;
    uint16_t attributes;
    uint32_t create;
    uint32_t last_modified;
    uint32_t last_accessed;
};

struct sede
{
    uint8_t entry_type;
    uint8_t secondary_flags;
    uint8_t name_length;
    uint32_t first_cluster;
    uint64_t data_length;
};

struct fnede
{
    uint8_t entry_type;
    uint8_t secondary_flags;
    uint16_t name[FNEDE_UNAME_LENGTH];
};

struct lfnde
{
    struct fde *fde;
    struct sede *sede;
    struct alist *fnede_list;
};

void lfnde_create(struct lfnde *e);
void lfnde_readbuf(uint8_t *buf, struct lfnde *e);
void lfnde_writebuf(struct lfnde *e, uint8_t *buf);
uint64_t lfnde_getdatalen(/*in*/ struct lfnde *e);
void lfnde_setdatalen(/*in*/ struct lfnde *e, /*in*/ uint64_t len);
void lfnde_getname(/*in*/ struct lfnde *e, /*out*/ char *name);
void lfnde_setname(/*in*/ struct lfnde *e, /*in*/ const char *name);
bool lfnde_isdir(/*in*/ struct lfnde *e);
bool lfnde_isfile(/*in*/ struct lfnde *e);
void lfnde_setisdir(/*in*/ struct lfnde *e, /*in*/ bool val);
uint32_t lfnde_getstartcluster(/*in*/ struct lfnde *e);
void lfnde_setstartcluster(/*in*/ struct lfnde *e, /*in*/ uint32_t start_cluster);
uint16_t lfnde_count(struct lfnde *e);
void lfnde_destruct(struct lfnde *e);


#endif /* VFAT_LFNDE_H */
