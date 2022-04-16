#include <errno.h>
#include <stdlib.h>

#include "../include/ClusterChainFile.h"
#include "../include/DirectoryEntry.h"

/**
 * @brief Returns the length of this file in bytes. This is the length that
 * is stored in the directory entry that is associated with this file.
 * @return
 */
uint32_t ClusterChainFile::GetLength() const
{
    return this->entry.GetDataLength();
}

void ClusterChainFile::SetLength(uint32_t val)
{
    this->chain.SetSizeInBytes(val);
    this->entry.SetStartCluster(this->chain.GetStartCluster());
    this->entry.SetDataLength(val);
}

/**
 * @brief Reads from this file into the specified buffer.
 * @param device
 * @param offset
 * @param nbytes
 * @param buffer
 * @return
 */
uint32_t ClusterChainFile::Read(const Device& device, uint32_t offset, uint32_t nbytes, uint8_t *buffer)
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
        this->chain.ReadData(device, offset, nbytes, buffer);
    }

    return nbytes;
}

void ClusterChainFile::Write(Device& device, uint32_t offset, uint32_t nbytes, uint8_t *buffer)
{
    uint32_t length = offset + nbytes;
    if (length > this->GetLength()) {
        this->SetLength(length);
    }

    this->chain.WriteData(device, offset, nbytes, buffer);
}

ClusterChainFile::ClusterChainFile(const DirectoryEntry& entry, ClusterChain& chain)
{
    this->entry = entry;
    this->chain = chain;
}

ClusterChainFile::~ClusterChainFile()
{
}
