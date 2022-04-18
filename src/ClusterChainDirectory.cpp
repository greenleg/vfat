#include <assert.h>
#include <stdexcept>
#include <sstream>
#include <iostream>

//#include "../include/string.h"
#include "../include/Common.h"
#include "../include/Fat.h"
#include "../include/ClusterChainDirectory.h"
#include "../include/ClusterChainFile.h"
#include "../include/DirectoryEntry.h"
#include "../include/BootSector.h"

using namespace org::vfat;

ClusterChainDirectory::ClusterChainDirectory() 
    : capacity(0), isRoot(false), isDirty(false)
{}

ClusterChainDirectory::ClusterChainDirectory(const ClusterChainDirectory& other) 
  : chain(other.chain), 
    entries(other.entries), 
    capacity(other.capacity), 
    isRoot(other.isRoot), 
    isDirty(other.isDirty) 
{ }

ClusterChainDirectory::ClusterChainDirectory(ClusterChainDirectory&& other)
  : chain(std::move(other.chain)), 
    entries(std::move(other.entries)),
    capacity(std::move(other.capacity)), 
    isRoot(std::move(other.isRoot)), 
    isDirty(std::move(other.isDirty))
{ }

ClusterChainDirectory& ClusterChainDirectory::operator=(const ClusterChainDirectory& other)
{
    if (this != &other) {
        Cleanup();
        
        chain = other.chain;
        entries = other.entries;
        capacity = other.capacity;
        isRoot = other.isRoot;
        isDirty = other.isDirty;
    }

    return *this;
}

ClusterChainDirectory& ClusterChainDirectory::operator=(ClusterChainDirectory&& other)
{
    if (this != &other) {
        Cleanup();
    
        chain = std::move(other.chain);
        entries = std::move(other.entries);
        capacity = std::move(other.capacity);
        isRoot = std::move(other.isRoot);
        isDirty = std::move(other.isDirty);
    }

    return *this;
}

void ClusterChainDirectory::Cleanup()
{
}

void ClusterChainDirectory::CheckUniqueName(const char *name)
{
    auto idx = this->FindEntryIndex(name);
    if (idx != -1) {
        std::ostringstream msgStream;
        msgStream << "Directory '" << name << "' already exists.";
        throw std::ios_base::failure(msgStream.str());
    }
}

void ClusterChainDirectory::ReadEntries(uint8_t *buffer)
{
    uint32_t offset = 0;
    size_t entryIdx = 0;
    while (entryIdx < this->capacity) {
        uint8_t entryType = buffer[offset];
        if (entryType == NO_DIR_ENTRY) {
            break;
        }

        DirectoryEntry e;
        e.Read(buffer + offset);
        offset += e.GetFat32EntryCount() * FAT_DIR_ENTRY_SIZE;
        entryIdx += e.GetFat32EntryCount();
        this->entries.push_back(std::move(e));
    }
}

uint32_t ClusterChainDirectory::WriteEntries(uint8_t *buffer, uint32_t bufferSize) const
{
    uint32_t offset = 0;
    for (uint32_t i = 0; i < this->entries.size(); ++i) {
        const DirectoryEntry& e = this->entries[i];
        e.Write(buffer + offset);
        offset += e.GetFat32EntryCount() * FAT_DIR_ENTRY_SIZE;
    }

    if (offset < bufferSize) {
        // Write the end-of-list marker.
        buffer[offset] = NO_DIR_ENTRY;
        offset += 1;
    }

    // `offset` is equal to number of the actually written bytes.
    return offset;
}

void ClusterChainDirectory::Read(const Device& device, const Fat& fat, uint32_t firstCluster, bool isRoot)
{

    ClusterChain cc(firstCluster);
    uint64_t size = cc.GetSizeInBytes(fat);
    uint8_t buffer[size];
    cc.ReadData(device, fat, 0, size, buffer);

    this->chain = std::move(cc);
    this->isRoot = isRoot;
    this->capacity = size / FAT_DIR_ENTRY_SIZE;

    this->ReadEntries(buffer);
}

void ClusterChainDirectory::Write(Device& device, Fat& fat)
{
    uint32_t nbytes = this->capacity * FAT_DIR_ENTRY_SIZE;
    uint8_t buffer[nbytes];
    uint32_t realBytes = this->WriteEntries(buffer, nbytes);
    this->chain.WriteData(device, fat, 0, realBytes, buffer);
}

void ClusterChainDirectory::Create(ClusterChain& cc, const Fat& fat)
{
    this->chain = cc;
    this->isRoot = false;
    this->capacity = cc.GetSizeInBytes(fat) / FAT_DIR_ENTRY_SIZE;
}

void ClusterChainDirectory::CreateRoot(Fat& fat)
{
    BootSector& bootSector = fat.GetBootSector();
    ClusterChain cc(0);
    cc.SetLength(fat, 1);
    bootSector.SetRootDirFirstCluster(cc.GetStartCluster());

    this->isRoot = true;
    this->capacity = cc.GetSizeInBytes(fat) / FAT_DIR_ENTRY_SIZE;
    this->chain = std::move(cc);
}

void ClusterChainDirectory::ReadRoot(const Device& device, const Fat& fat)
{
    this->Read(device, fat, fat.GetBootSector().GetRootDirFirstCluster(), true);
}

void ClusterChainDirectory::ChangeSize(Fat& fat, uint32_t fat32EntryCount)
{
    uint32_t size = fat32EntryCount * FAT_DIR_ENTRY_SIZE;
    uint32_t newSize = this->chain.SetSizeInBytes(fat, size);
    this->capacity = newSize / FAT_DIR_ENTRY_SIZE;
}

uint32_t ClusterChainDirectory::GetFat32EntryCount() const
{
    uint32_t n = 0;
    for (uint32_t i = 0; i < this->entries.size(); ++i) {
        const DirectoryEntry& e = this->entries[i];
        n += e.GetFat32EntryCount();
    }

    return n;
}

DirectoryEntry& ClusterChainDirectory::AddEntry(DirectoryEntry& e, Fat& fat)
{
    uint32_t newCount = this->GetFat32EntryCount() + e.GetFat32EntryCount();
    if (newCount > this->capacity) {
        this->ChangeSize(fat, newCount);
    }

    this->entries.push_back(e);
    return this->entries.back();
}

DirectoryEntry& ClusterChainDirectory::GetEntry(uint32_t index)
{
    return this->entries[index];
}

DirectoryEntry& ClusterChainDirectory::FindEntry(const char *name)
{
    char nameBuf[256];
    for (uint32_t i = 0; i < this->entries.size(); ++i) {
        DirectoryEntry& e = this->entries[i];
        e.GetName(nameBuf);
        if (strcmp(name, nameBuf) == 0) {
            return e;
        }
    }

    /* not found */
    std::ostringstream msgStream;
    msgStream << "Entry '" << name << "' not found.";
    throw std::ios_base::failure(msgStream.str());
}

const DirectoryEntry& ClusterChainDirectory::FindEntry(const char *name) const
{
    char nameBuf[256];
    for (uint32_t i = 0; i < this->entries.size(); ++i) {
        const DirectoryEntry& e = this->entries[i];
        e.GetName(nameBuf);
        if (strcmp(name, nameBuf) == 0) {
            return e;
        }
    }

    std::ostringstream msgStream;
    msgStream << "Entry '" << name << "' not found.";
    throw std::ios_base::failure(msgStream.str());
}

int32_t ClusterChainDirectory::FindEntryIndex(const char *name)
{
    char nameBuf[256];
    for (uint32_t i = 0; i < this->entries.size(); ++i) {
        DirectoryEntry& e = this->entries[i];
        e.GetName(nameBuf);
        if (strcmp(name, nameBuf) == 0) {
            return i;
        }
    }

    /* not found */
    return -1;
}

void ClusterChainDirectory::RemoveEntry(Fat& fat, uint32_t index)
{
    this->entries.erase(this->entries.begin() + index);
    uint32_t newCount = this->GetFat32EntryCount();
    if (newCount > 0) {
        this->ChangeSize(fat, newCount);
    } else {
        this->ChangeSize(fat, 1); // Empty directory consists of 1 cluster
    }
}

DirectoryEntry& ClusterChainDirectory::AddDirectory(const char *name, Device& device, Fat& fat)
{
    this->CheckUniqueName(name);

    ClusterChain cc(0);
    cc.SetLength(fat, 1);

    time_t now = time(0);

    DirectoryEntry subde;
    subde.SetName(name);
    subde.SetIsDir(true);
    subde.SetStartCluster(cc.GetStartCluster());
    subde.SetCreatedTime(now);
    subde.SetLastModifiedTime(now);

    this->AddEntry(subde, fat);
    this->Write(device, fat);

    ClusterChainDirectory subDir;
    subDir.Create(cc, fat);

    // Add `.` entry
    DirectoryEntry dot;
    dot.SetName(".");
    dot.SetIsDir(true);
    dot.SetStartCluster(subDir.chain.GetStartCluster());
    dot.SetCreatedTime(now);
    dot.SetLastModifiedTime(now);
    subDir.AddEntry(dot, fat);

    // Add `..` entry
    DirectoryEntry dotdot;
    dotdot.SetName("..");
    dotdot.SetIsDir(true);
    dotdot.SetStartCluster(this->chain.GetStartCluster());

    // TODO: copy date/time fields from entry to dotdot;

    subDir.AddEntry(dotdot, fat);
    subDir.Write(device, fat);

    return this->entries.back();   // subde
}

void ClusterChainDirectory::RemoveDirectory(const char *name, Device& device, Fat& fat)
{
    uint32_t index = this->FindEntryIndex(name);
    if (index < 0) {
        std::ostringstream msgStream;
        msgStream << "Couldn't find '" << name << "': No such file or directory.";
        throw std::ios_base::failure(msgStream.str());
    }

    return this->RemoveDirectory(index, device, fat);
}

void ClusterChainDirectory::RemoveDirectory(uint32_t index, Device& device, Fat& fat)
{
    DirectoryEntry& e = this->GetEntry(index);
    char nameBuf[256];
    e.GetName(nameBuf);
    if (strcmp(nameBuf, ".") == 0 || strcmp(nameBuf, "..") == 0) {
        return;
    }

    ClusterChainDirectory subDir = ClusterChainDirectory::GetDirectory(device, fat, e);

    for (size_t i = 0; i < subDir.GetEntries().size(); ++i) {
        DirectoryEntry& subde = subDir.GetEntry(i);
        if (subde.IsDir()) {
            subDir.RemoveDirectory(i, device, fat);
        } else {
            subDir.RemoveFile(i, device, fat);
        }
    }

    ClusterChain cc(e.GetStartCluster());
    cc.SetLength(fat, 0);

    this->RemoveEntry(fat, index);
    this->Write(device, fat);
}

void ClusterChainDirectory::RemoveFile(const char *name, Device& device, Fat& fat)
{
    uint32_t index = this->FindEntryIndex(name);
    if (index < 0) {
        std::ostringstream msgStream;
        msgStream << "Couldn't find '" << name << "': No such file or directory.";
        throw std::runtime_error(msgStream.str());
    }

    this->RemoveFile(index, device, fat);
}

void ClusterChainDirectory::RemoveFile(uint32_t index, Device& device, Fat& fat)
{
    DirectoryEntry e = this->GetEntry(index);
    ClusterChain cc(e.GetStartCluster());
    cc.SetLength(fat, 0);

    this->RemoveEntry(fat, index);
    this->Write(device, fat);
}

DirectoryEntry& ClusterChainDirectory::AddFile(const char *name, Device& device, Fat& fat)
{
    this->CheckUniqueName(name);

    time_t now = time(0);

    DirectoryEntry e;
    e.SetName(name);
    e.SetIsDir(false);
    e.SetStartCluster(0);
    e.SetDataLength(0);
    e.SetCreatedTime(now);
    e.SetLastModifiedTime(now);

    this->AddEntry(e, fat);
    this->Write(device, fat);

    return this->entries.back();
}

ClusterChainFile ClusterChainDirectory::GetFile(const DirectoryEntry& e)
{
    ClusterChain cc(e.GetStartCluster());
    ClusterChainFile file(e, cc);
    return file;
}

ClusterChainDirectory ClusterChainDirectory::GetDirectory(const Device& device, const Fat& fat, const DirectoryEntry& e)
{
    uint32_t firstCluster = e.GetStartCluster();
    ClusterChainDirectory dir;
    dir.Read(device, fat, firstCluster, false);

    return dir;
}

void ClusterChainDirectory::Move(Device& device, Fat& fat, DirectoryEntry& e, ClusterChainDirectory& dest, const char *newName)
{
    dest.CheckUniqueName(newName);

    char oldName[256];
    e.GetName(oldName);

    uint32_t index = this->FindEntryIndex(oldName);
    
    // Get a copy of entry before removing
    DirectoryEntry copyEntry = e;
    
    this->RemoveEntry(fat, index);
    this->Write(device, fat);

    copyEntry.SetName(newName);
    DirectoryEntry& newEntry = dest.AddEntry(copyEntry, fat);
    dest.Write(device, fat);


    if (e.IsDir()) {
        ClusterChainDirectory dir = GetDirectory(device, fat, newEntry);
        DirectoryEntry& dotdot = dir.FindEntry("..");
        assert(dotdot.GetStartCluster() == this->chain.GetStartCluster());
        dotdot.SetStartCluster(dest.chain.GetStartCluster());
        
        // Write changes to the disk;
        dir.Write(device, fat);
    }
}

void ClusterChainDirectory::CopyDirectory(Device& device, Fat& fat, DirectoryEntry& e, ClusterChainDirectory& dest) const
{
    char nameBuf[256];
    e.GetName(nameBuf);

    this->CopyDirectory(device, fat, e, dest, nameBuf);
}

void ClusterChainDirectory::CopyDirectory(Device& device, Fat& fat, DirectoryEntry& e, ClusterChainDirectory& dest, const char *newName) const
{
    assert(e.IsDir());

    ClusterChainDirectory orig = GetDirectory(device, fat, e);
    DirectoryEntry copye = dest.AddDirectory(newName, device, fat);
    ClusterChainDirectory copy = GetDirectory(device, fat, copye);

    char nameBuf[256];
    for (uint32_t i = 0; i < orig.entries.size(); ++i) {
        DirectoryEntry& child = orig.entries[i];
        if (child.IsDir()) {
            child.GetName(nameBuf);
            if (strcmp(nameBuf, ".") != 0 && strcmp(nameBuf, "..") != 0) {
                orig.CopyDirectory(device, fat, child, copy);
            }
        } else {
            orig.CopyFile(device, fat, child, copy);
        }
    }

    copy.Write(device, fat);
}

ClusterChainFile ClusterChainDirectory::CopyFile(Device& device, Fat& fat, DirectoryEntry& e, ClusterChainDirectory& dest) const
{
    char nameBuf[256];
    e.GetName(nameBuf);

    return this->CopyFile(device, fat, e, dest, nameBuf);
}

ClusterChainFile ClusterChainDirectory::CopyFile(Device& device, Fat& fat, DirectoryEntry& e, ClusterChainDirectory& dest, const char *newName) const
{
    assert(e.IsFile());

    DirectoryEntry& copye = dest.AddFile(newName, device, fat);

    // Copy file content via buffer
    const int bufferSize = 4096;
    uint8_t buf[bufferSize];

    ClusterChainFile orig = GetFile(e);
    ClusterChainFile copy = dest.GetFile(copye);

    uint32_t pos = 0;
    uint32_t nread = orig.Read(device, fat, pos, bufferSize, buf);
    while (nread > 0) {
        copy.Write(device, fat, pos, nread, buf);
        pos += nread;
        nread = orig.Read(device, fat, pos, bufferSize, buf);
    }
    
    copye = copy.GetEntry();
    
    return copy;
}

void ClusterChainDirectory::SetName(Device& device, Fat& fat, DirectoryEntry& e, const char *name)
{
    this->Move(device, fat, e, *this, name);
}

ClusterChainDirectory::~ClusterChainDirectory()
{
    Cleanup();
}
