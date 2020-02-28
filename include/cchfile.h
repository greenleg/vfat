#ifndef VFAT_CCHFILE_H
#define VFAT_CCHFILE_H

#include "cch.h"

struct cchfile
{
    struct lfnde *entry;
    struct cch *chain;
};

/*
 * Returns the length of this file in bytes. This is the length that
 * is stored in the directory entry that is associated with this file.
 */
u32 cchfile_getlen(/*in*/ struct cchfile *file);

void cchfile_setlen(/*in*/ struct cchfile *file, /*in*/ u32 len);

/*
 * Reads from this file into the specified buffer.
 */
bool cchfile_read(/*in*/ struct fdisk *dev,
                  /*in*/ struct cchfile *file,
                  /*in*/ u32 offset,
                  /*in*/ u32 nbytes,
                  /*out*/ u32 *nread,
                  /*out*/ u8 *buf);

void cchfile_write(/*in*/ struct fdisk *dev,
                   /*in*/ struct cchfile *file,
                   /*in*/ u32 offset,
                   /*in*/ u32 nbytes,
                   /*in*/ u8 *buf);

void cchfile_destruct(/*in*/ struct cchfile *file);

#endif
