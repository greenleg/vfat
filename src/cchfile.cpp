#include <errno.h>
#include <stdlib.h>

#include "../include/cchfile.h"
#include "../include/DirectoryEntry.h"

///*
// * Returns the length of this file in bytes. This is the length that
// * is stored in the directory entry that is associated with this file.
// */
//uint32_t cchfile_getlen(/*in*/ struct cchfile *file)
//{
//    return file->entry->GetDataLength();
//}

uint32_t ClusterChainFile::GetLength() const
{
    return this->entry->GetDataLength();
}

//void cchfile_setlen(/*in*/ struct cchfile *file, /*in*/ uint32_t len)
//{
//    file->chain->SetSizeInBytes(len);
//    file->entry->SetStartCluster(file->chain->GetStartCluster());
//    file->entry->SetDataLength(len);
//}

void ClusterChainFile::SetLength(uint32_t val)
{
    this->chain->SetSizeInBytes(val);
    this->entry->SetStartCluster(this->chain->GetStartCluster());
    this->entry->SetDataLength(val);
}

///*
// * Reads from this file into the specified buffer.
// */
//bool cchfile_read(/*in*/ org::vfat::FileDisk *device,
//                  /*in*/ struct cchfile *file,
//                  /*in*/ uint32_t offset,
//                  /*in*/ uint32_t nbytes,
//                  /*out*/ uint32_t *nread,
//                  /*out*/ uint8_t *buffer)
//{
//    uint32_t datalen = cchfile_getlen(file);
//    if (offset + nbytes > datalen) {
//        // The file offset is beyond the end of the file.
//        if (offset < datalen) {
//            nbytes = datalen - offset;
//        } else {
//            nbytes = 0;
//        }
//    }

//    if (nbytes > 0) {
//        file->chain->ReadData(device, offset, nbytes, buffer);
//    }

//    *nread = nbytes;
//    return true;
//}

uint32_t ClusterChainFile::Read(FileDisk *device, uint32_t offset, uint32_t nbytes, uint8_t *buffer)
{
    uint32_t dataLength = this->GetLength();
    if (offset + nbytes > dataLength) {
        // The file offset is beyond the end of the file.
        if (offset < dataLength) {
            nbytes = dataLength - offset;
        } else {
            nbytes = 0;
        }
    }

    if (nbytes > 0) {
        this->chain->ReadData(device, offset, nbytes, buffer);
    }

    return nbytes;
}

//void cchfile_write(/*in*/ org::vfat::FileDisk *device,
//                   /*in*/ struct cchfile *file,
//                   /*in*/ uint32_t offset,
//                   /*in*/ uint32_t nbytes,
//                   /*in*/ uint8_t *buffer)
//{
//    uint32_t lastByte = offset + nbytes;
//    if (lastByte > cchfile_getlen(file)) {
//        cchfile_setlen(file, lastByte);
//    }

//    file->chain->WriteData(device, offset, nbytes, buffer);
//}

void ClusterChainFile::Write(FileDisk *device, uint32_t offset, uint32_t nbytes, uint8_t *buffer)
{
    uint32_t length = offset + nbytes;
    if (length > this->GetLength()) {
        this->SetLength(length);
    }

    this->chain->WriteData(device, offset, nbytes, buffer);
}

//void cchfile_destruct(/*in*/ struct cchfile *file)
//{
//    free(file->chain);
//}

ClusterChainFile::ClusterChainFile(DirectoryEntry *entry, ClusterChain *chain)
{
    this->entry = entry;
    this->chain = chain;
}

ClusterChainFile::~ClusterChainFile()
{
    delete this->chain;
}
