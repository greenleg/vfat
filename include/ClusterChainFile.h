#ifndef VFAT_CCHFILE_H
#define VFAT_CCHFILE_H

#include "ClusterChain.h"
#include "DirectoryEntry.h"

namespace org::vfat
{
    class ClusterChainFile
    {
    private:
        DirectoryEntry entry;
        ClusterChain chain;

    public:
        ClusterChainFile() {}
        
        ClusterChainFile(const ClusterChainFile& other)
            : entry(other.entry), chain(other.chain)
        {}
        
        ClusterChainFile(ClusterChainFile&& other)
            : entry(std::move(other.entry)), chain(std::move(other.chain))
        {}
        
        ClusterChainFile& operator=(const ClusterChainFile& other)
        {
            if (this != &other) {
                entry = other.entry;
                chain = other.chain;
            }
            return *this;
        }
        
        ClusterChainFile& operator=(ClusterChainFile&& other)
        {
            if (this != &other) {
                entry = std::move(other.entry);
                chain = std::move(other.chain);
            }
            return *this;
        }
        
        ClusterChainFile(const DirectoryEntry& entry, ClusterChain& chain);

        /**
         * @brief Gets the length of this file in bytes.
         * @return A length that is stored in the directory entry that is associated with this file.
         */
        uint32_t GetLength() const;
        void SetLength(Fat& fat, uint32_t val);

        /**
         * @brief Reads from this file into the specified buffer.
         * @param device
         * @param offset
         * @param nbytes
         * @param buffer
         * @return
         */
        uint32_t Read(const Device& device, const Fat& fat, uint32_t offset, uint32_t nbytes, uint8_t *buffer) const;

        void Write(Device& device, Fat& fat, uint32_t offset, uint32_t nbytes, uint8_t *buffer);
        
        const DirectoryEntry& GetEntry() const { return this->entry; }

        ~ClusterChainFile();
    };
}

#endif
