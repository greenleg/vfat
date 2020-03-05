#include <errno.h>
#include <stdlib.h>

#include "../include/cchfile.h"
#include "../include/lfnde.h"

/*
 * Returns the length of this file in bytes. This is the length that
 * is stored in the directory entry that is associated with this file.
 */
uint32_t cchfile_getlen(/*in*/ struct cchfile *file)
{
    return file->entry->sede->data_length;
}

void cchfile_setlen(/*in*/ struct cchfile *file, /*in*/ uint32_t len)
{
    cch_setsize(file->chain, len);
    lfnde_setstartcluster(file->entry, file->chain->start_cluster);
    lfnde_setdatalen(file->entry, len);
}

/*
 * Reads from this file into the specified buffer.
 */
bool cchfile_read(/*in*/ org::vfat::FileDisk *device,
                  /*in*/ struct cchfile *file,
                  /*in*/ uint32_t offset,
                  /*in*/ uint32_t nbytes,
                  /*out*/ uint32_t *nread,
                  /*out*/ uint8_t *buffer)
{
    uint32_t datalen = cchfile_getlen(file);
    if (offset + nbytes > datalen) {
        // The file offset is beyond the end of the file.
        if (offset < datalen) {
            nbytes = datalen - offset;
        } else {
            nbytes = 0;
        }
    }    

    if (nbytes > 0) {
        cch_readdata(device, file->chain, offset, nbytes, buffer);
    }

    *nread = nbytes;
    return true;
}

void cchfile_write(/*in*/ org::vfat::FileDisk *device,
                   /*in*/ struct cchfile *file,
                   /*in*/ uint32_t offset,
                   /*in*/ uint32_t nbytes,
                   /*in*/ uint8_t *buffer)
{
    uint32_t lastByte = offset + nbytes;
    if (lastByte > cchfile_getlen(file)) {
        cchfile_setlen(file, lastByte);
    }

    cch_writedata(device, file->chain, offset, nbytes, buffer);
}

void cchfile_destruct(/*in*/ struct cchfile *file)
{
    free(file->chain);
}
