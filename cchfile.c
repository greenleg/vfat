#include <errno.h>
#include <stdlib.h>

#include "cchfile.h"
#include "lfnde.h"

/*
 * Returns the length of this file in bytes. This is the length that
 * is stored in the directory entry that is associated with this file.
 */
u32 cchfile_getlen(/*in*/ struct cchfile *file)
{
    return file->entry->sede->data_length;
}

void cchfile_setlen(/*in*/ struct cchfile *file, /*in*/ u32 len)
{
    cch_setsize(file->chain, len);
    lfnde_setstartcluster(file->entry, file->chain->start_cluster);
    lfnde_setdatalen(file->entry, len);
}

/*
 * Reads from this file into the specified buffer.
 */
bool cchfile_read(/*in*/ struct fdisk *dev,
                  /*in*/ struct cchfile *file,
                  /*in*/ u32 offset,
                  /*in*/ u32 nbytes,
                  /*out*/ u8 *buf)
{
    if (nbytes == 0) {
        return true;
    }

    if (offset + nbytes > cchfile_getlen(file)) {
        // The file offset is beyond the end of the file.
        vfat_errno = EIO;
        return false;
    }

    cch_readdata(dev, file->chain, offset, nbytes, buf);
    return true;
}

void cchfile_write(/*in*/ struct fdisk *dev,
                   /*in*/ struct cchfile *file,
                   /*in*/ u32 offset,
                   /*in*/ u32 nbytes,
                   /*in*/ u8 *buf)
{
    u32 lastByte = offset + nbytes;
    if (lastByte > cchfile_getlen(file)) {
        cchfile_setlen(file, lastByte);
    }

    cch_writedata(dev, file->chain, offset, nbytes, buf);
}

void cchfile_destruct(/*in*/ struct cchfile *file)
{
    free(file->chain);
}
