#ifndef VFAT_CCHFILE_H
#define VFAT_CCHFILE_H

#include "ClusterChain.h"

struct cchfile
{
    struct lfnde *entry;
    struct ClusterChain *chain;
};

/*
 * Returns the length of this file in bytes. This is the length that
 * is stored in the directory entry that is associated with this file.
 */
uint32_t cchfile_getlen(/*in*/ struct cchfile *file);

void cchfile_setlen(/*in*/ struct cchfile *file, /*in*/ uint32_t len);

/*
 * Reads from this file into the specified buffer.
 */
bool cchfile_read(/*in*/ org::vfat::FileDisk *device,
                  /*in*/ struct cchfile *file,
                  /*in*/ uint32_t offset,
                  /*in*/ uint32_t nbytes,
                  /*out*/ uint32_t *nread,
                  /*out*/ uint8_t *buf);

void cchfile_write(/*in*/ org::vfat::FileDisk *device,
                   /*in*/ struct cchfile *file,
                   /*in*/ uint32_t offset,
                   /*in*/ uint32_t nbytes,
                   /*in*/ uint8_t *buf);

void cchfile_destruct(/*in*/ struct cchfile *file);

#endif
