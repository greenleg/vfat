#ifndef VFAT_CCHFILE_H
#define VFAT_CCHFILE_H

#include "ClusterChain.h"
#include "DirectoryEntry.h"

using namespace org::vfat;

//struct cchfile
//{
//    DirectoryEntry *entry;
//    ClusterChain *chain;
//};

/*
 * Returns the length of this file in bytes. This is the length that is stored in the directory entry that is associated with this file.
 */
//uint32_t cchfile_getlen(/*in*/ struct cchfile *file);

//void cchfile_setlen(/*in*/ struct cchfile *file, /*in*/ uint32_t len);

/*
 * Reads from this file into the specified buffer.
 */
//bool cchfile_read(/*in*/ org::vfat::FileDisk *device,
//                  /*in*/ struct cchfile *file,
//                  /*in*/ uint32_t offset,
//                  /*in*/ uint32_t nbytes,
//                  /*out*/ uint32_t *nread,
//                  /*out*/ uint8_t *buf);

//void cchfile_write(/*in*/ org::vfat::FileDisk *device,
//                   /*in*/ struct cchfile *file,
//                   /*in*/ uint32_t offset,
//                   /*in*/ uint32_t nbytes,
//                   /*in*/ uint8_t *buf);

//void cchfile_destruct(/*in*/ struct cchfile *file);

namespace org::vfat
{
    class ClusterChainFile
    {
    private:
        DirectoryEntry *entry;
        ClusterChain *chain;

    public:
        ClusterChainFile(DirectoryEntry *entry, ClusterChain *chain);

        /**
         * @brief Gets the length of this file in bytes.
         * @return A length that is stored in the directory entry that is associated with this file.
         */
        uint32_t GetLength() const;
        void SetLength(uint32_t val);

        /**
         * @brief Reads from this file into the specified buffer.
         * @param device
         * @param offset
         * @param nbytes
         * @param buffer
         * @return
         */
        uint32_t Read(FileDisk *device, uint32_t offset, uint32_t nbytes, uint8_t *buffer);

        void Write(FileDisk *device, uint32_t offset, uint32_t nbytes, uint8_t *buffer);

        ~ClusterChainFile();
    };
}

#endif
