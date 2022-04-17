#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <iostream>

#include "../include/Common.h"
#include "../include/BinaryReader.h"
#include "../include/DirectoryEntry.h"

using namespace org::vfat;

#define FDE_ENTRYTYPE_OFFSET 0
#define FDE_ATTRIBUTES_OFFSET 4
#define FDE_CREATED_OFFSET 8
#define FDE_LASTMODIFIED_OFFSET 12
#define FDE_LASTACCESSED_OFFSET 16
#define FDE_NAMELENGTH_OFFSET 20
#define FDE_FIRSTCLUSTER_OFFSET 21
#define FDE_DATALENGTH_OFFSET 25

#define FNDE_ENTRYTYPE_OFFSET 0
#define FNDE_FILENAME_OFFSET 2

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

DirectoryEntry::DirectoryEntry()
    : pImpl(std::make_unique<DirectoryEntryImpl>())
{
}

DirectoryEntry::DirectoryEntry(const DirectoryEntry& other)
    : pImpl(std::make_unique<DirectoryEntryImpl>(*other.pImpl))
{
}

DirectoryEntry::DirectoryEntry(DirectoryEntry&& other)
   : pImpl(std::move(other.pImpl))
{
}

DirectoryEntry& DirectoryEntry::operator=(const DirectoryEntry& other)
{
    if (this != &other) {
        pImpl = std::make_unique<DirectoryEntryImpl>(*other.pImpl);
    }
    return *this;
}

DirectoryEntry& DirectoryEntry::operator=(DirectoryEntry&& other)
{
    if (this != &other) {
        pImpl = std::move(other.pImpl);
    }
    return *this;
}

DirectoryEntry::~DirectoryEntry()
{
}

uint16_t DirectoryEntryImpl::GetFat32EntryCount() const
{
    return 1 + this->fndeList.size();
}

void DirectoryEntryImpl::Read(uint8_t *buffer)
{
    uint8_t entryType = BinaryReader::ReadUInt8(buffer, FDE_ENTRYTYPE_OFFSET);
    assert(entryType == BASE_DIR_ENTRY);

    this->attributes = BinaryReader::ReadUInt16(buffer, FDE_ATTRIBUTES_OFFSET);
    this->created = BinaryReader::ReadUInt32(buffer, FDE_CREATED_OFFSET);
    this->lastModified = BinaryReader::ReadUInt32(buffer, FDE_LASTMODIFIED_OFFSET);
    this->lastAccessed = BinaryReader::ReadUInt32(buffer, FDE_LASTACCESSED_OFFSET);
    this->nameLength = BinaryReader::ReadUInt8(buffer, FDE_NAMELENGTH_OFFSET);
    this->firstCluster = BinaryReader::ReadUInt32(buffer, FDE_FIRSTCLUSTER_OFFSET);
    this->dataLength = BinaryReader::ReadUInt32(buffer, FDE_DATALENGTH_OFFSET);

    buffer += FAT_DIR_ENTRY_SIZE;

    int fndeCount = (this->nameLength + FNDE_NAME_LENGTH - 1) / FNDE_NAME_LENGTH;
    for (uint8_t i = 0; i < fndeCount; ++i) {
        uint8_t entryType = BinaryReader::ReadUInt8(buffer, FNDE_ENTRYTYPE_OFFSET);
        assert(entryType == FILENAME_DIR_ENTRY);

        struct FileNameDirectoryEntry fnde;
        memcpy(fnde.nameBuffer, buffer + FNDE_FILENAME_OFFSET, FNDE_NAME_LENGTH);
        this->fndeList.push_back(std::move(fnde));

        buffer += FAT_DIR_ENTRY_SIZE;
    }
}

void DirectoryEntryImpl::Write(uint8_t *buffer) const
{
    uint8_t fndeCount = (this->nameLength + FNDE_NAME_LENGTH - 1) / FNDE_NAME_LENGTH;
    assert(this->fndeList->size() == fndeCount);

    BinaryReader::WriteUInt8(buffer, FDE_ENTRYTYPE_OFFSET, BASE_DIR_ENTRY);
    BinaryReader::WriteUInt16(buffer, FDE_ATTRIBUTES_OFFSET, this->attributes);
    BinaryReader::WriteUInt32(buffer, FDE_CREATED_OFFSET, this->created);
    BinaryReader::WriteUInt32(buffer, FDE_LASTMODIFIED_OFFSET, this->lastModified);
    BinaryReader::WriteUInt32(buffer, FDE_LASTACCESSED_OFFSET, this->lastAccessed);
    BinaryReader::WriteUInt8(buffer, FDE_NAMELENGTH_OFFSET, this->nameLength);
    BinaryReader::WriteUInt32(buffer, FDE_FIRSTCLUSTER_OFFSET, this->firstCluster);
    BinaryReader::WriteUInt32(buffer, FDE_DATALENGTH_OFFSET, this->dataLength);

    buffer += FAT_DIR_ENTRY_SIZE;

    for (uint8_t i = 0; i < this->fndeList.size(); ++i) {
        const FileNameDirectoryEntry& fnde = this->fndeList[i];
        BinaryReader::WriteUInt8(buffer, FNDE_ENTRYTYPE_OFFSET, FILENAME_DIR_ENTRY);
        memcpy(buffer + FNDE_FILENAME_OFFSET, fnde.nameBuffer, FNDE_NAME_LENGTH);

        buffer += FAT_DIR_ENTRY_SIZE;
    }
}

bool DirectoryEntryImpl::IsDir() const
{
    uint16_t attr = this->attributes;
    return (attr & DIRECTORY_MASK) != 0;
}

bool DirectoryEntryImpl::IsFile() const
{
    uint16_t attr = this->attributes;
    return (attr & DIRECTORY_MASK) == 0;
}

void DirectoryEntryImpl::SetIsDir(bool val)
{
    uint16_t attr = this->attributes;
    if (val) {
        attr |= DIRECTORY_MASK;
    } else {
        attr &= ~DIRECTORY_MASK;
    }

    this->attributes = attr;
}

uint32_t DirectoryEntryImpl::GetDataLength() const
{
    return this->dataLength;
}

void DirectoryEntryImpl::SetDataLength(uint32_t val)
{
    this->dataLength = val;
}

uint32_t DirectoryEntryImpl::GetStartCluster() const
{
    return this->firstCluster;
}

void DirectoryEntryImpl::SetStartCluster(uint32_t val)
{
    this->firstCluster = val;
}

void DirectoryEntryImpl::GetName(/*out*/ char *name) const
{
    uint8_t fndeCount = (this->nameLength + FNDE_NAME_LENGTH - 1) / FNDE_NAME_LENGTH;
    assert(this->fndeList->size() == fndeCount);

    uint8_t fndeIdx;
    uint8_t charIdx = 0;
    for (fndeIdx = 0; fndeIdx < fndeCount - 1; ++fndeIdx) {
        const FileNameDirectoryEntry& fnde = this->fndeList[fndeIdx];
        for (uint8_t i = 0; i < FNDE_NAME_LENGTH; ++i) {
            name[charIdx + i] = fnde.nameBuffer[i];
        }

        charIdx += FNDE_NAME_LENGTH;
    }

    const FileNameDirectoryEntry& fnde = this->fndeList[fndeIdx];
    for (uint8_t i = 0; i < this->nameLength - charIdx; ++i) {
        name[charIdx + i] = fnde.nameBuffer[i];
    }

    name[this->nameLength] = '\0';
}

void DirectoryEntryImpl::SetName(const char *name)
{
    uint8_t len = strlen(name);
    uint8_t fndeCount = this->fndeList.size();
    uint8_t newFndeCount = (len + (FNDE_NAME_LENGTH - 1)) / FNDE_NAME_LENGTH;

    // Clear list
    this->fndeList.clear();

    // Fill the list with the new values
    uint8_t charIdx = 0;
    for (uint8_t fndeIdx = 0; fndeIdx < newFndeCount - 1; ++fndeIdx) {
        FileNameDirectoryEntry fnde;
        for (uint8_t i = 0; i < FNDE_NAME_LENGTH; ++i) {
            fnde.nameBuffer[i] = name[charIdx + i];
        }

        charIdx += FNDE_NAME_LENGTH;
        this->fndeList.push_back(std::move(fnde));
    }

    // Special case for the last item
    FileNameDirectoryEntry fnde;
    for (uint8_t i = 0; i < len - charIdx; ++i) {
        fnde.nameBuffer[i] = name[charIdx + i];
    }

    this->fndeList.push_back(fnde);
    this->nameLength = len;
}

time_t DirectoryEntryImpl::GetCreatedTime() const
{
    return (time_t) this->created;
}

void DirectoryEntryImpl::SetCreatedTime(time_t time)
{
    this->created = (uint32_t) time;
}

time_t DirectoryEntryImpl::GetLastModifiedTime() const
{
    return (time_t) this->lastModified;
}

void DirectoryEntryImpl::SetLastModifiedTime(time_t time)
{
    this->lastModified = (uint32_t) time;
}
