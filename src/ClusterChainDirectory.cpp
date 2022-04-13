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
    for (size_t i = 0; i < this->entries.size(); ++i) {
        DirectoryEntry *e = this->entries[i];
//std::cout << "delete e=" << e << std::endl;
        delete e;
    }
}

void ClusterChainDirectory::CheckUniqueName(const char *name)
{
    if (this->FindEntry(name) != nullptr) {
        throw std::runtime_error("Directory already exists.");
    }
}

void ClusterChainDirectory::ReadEntries(uint8_t *buffer)
{
    uint32_t offset = 0;
    for (uint32_t i = 0; i < this->capacity; ++i) {
        uint8_t entryType = buffer[offset];
        if (entryType == NO_DIR_ENTRY) {
            break;
        }

        DirectoryEntry *e = new DirectoryEntry();
        e->Read(buffer + offset);
        this->entries.push_back(e);

        offset += e->GetFat32EntryCount() * FAT_DIR_ENTRY_SIZE;
    }
}

uint32_t ClusterChainDirectory::WriteEntries(uint8_t *buffer, uint32_t bufferSize) const
{
    uint32_t offset = 0;
    for (uint32_t i = 0; i < this->entries.size(); ++i) {
        DirectoryEntry *e = this->entries[i];
        e->Write(buffer + offset);
        offset += e->GetFat32EntryCount() * FAT_DIR_ENTRY_SIZE;
    }

    if (offset < bufferSize) {
        // Write the end-of-list marker.
        buffer[offset] = NO_DIR_ENTRY;
        offset += 1;
    }

    // `offset` is equal to number of the actually written bytes.
    return offset;
}

void ClusterChainDirectory::FormatDevice(Device * device, uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorPerCluster)
{
    BootSector bootSector;
    bootSector.Create(volumeSize, bytesPerSector, sectorPerCluster);
    bootSector.Write(device);

    Fat fat(&bootSector);
    fat.Create();

    ClusterChainDirectory root;
    root.CreateRoot(&fat);

    root.Write(device);
    fat.Write(device);
}

void ClusterChainDirectory::Read(Device *device, Fat *fat, uint32_t firstCluster, bool isRoot)
{
    ClusterChain cc(fat, firstCluster);

    uint64_t size = cc.GetSizeInBytes();
    uint8_t buffer[size];
    cc.ReadData(device, 0, size, buffer);

    this->chain = std::move(cc);
    this->isRoot = isRoot;
    this->capacity = size / FAT_DIR_ENTRY_SIZE;
    this->ReadEntries(buffer);
}

void ClusterChainDirectory::Write(Device *device)
{
    uint32_t nbytes = this->capacity * FAT_DIR_ENTRY_SIZE;
    uint8_t buffer[nbytes];
    uint32_t realBytes = this->WriteEntries(buffer, nbytes);
    this->chain.WriteData(device, 0, realBytes, buffer);
}

void ClusterChainDirectory::Create(ClusterChain& cc)
{
    this->chain = cc;
    this->isRoot = false;
    this->capacity = cc.GetSizeInBytes() / FAT_DIR_ENTRY_SIZE;
}

void ClusterChainDirectory::CreateRoot(Fat *fat)
{
    BootSector *bootSector = fat->GetBootSector();
    ClusterChain cc(fat, 0);
    cc.SetLength(1);
    bootSector->SetRootDirFirstCluster(cc.GetStartCluster());

    this->isRoot = true;
    this->capacity = cc.GetSizeInBytes() / FAT_DIR_ENTRY_SIZE;
    this->chain = std::move(cc);
}

void ClusterChainDirectory::ReadRoot(Device *device, Fat *fat)
{
    this->Read(device, fat, fat->GetBootSector()->GetRootDirFirstCluster(), true);
}

void ClusterChainDirectory::ChangeSize(uint32_t fat32EntryCount)
{
    uint32_t size = fat32EntryCount * FAT_DIR_ENTRY_SIZE;
    uint32_t newSize = this->chain.SetSizeInBytes(size);
    this->capacity = newSize / FAT_DIR_ENTRY_SIZE;
}

uint32_t ClusterChainDirectory::GetFat32EntryCount() const
{
    uint32_t n = 0;
    for (uint32_t i = 0; i < this->entries.size(); ++i) {
        DirectoryEntry *e = this->entries[i];
        n += e->GetFat32EntryCount();
    }

    return n;
}

void ClusterChainDirectory::AddEntry(DirectoryEntry *e)
{
    uint32_t newCount = this->GetFat32EntryCount() + e->GetFat32EntryCount();
    if (newCount > this->capacity) {
        this->ChangeSize(newCount);
    }

    this->entries.push_back(e);
}

DirectoryEntry * ClusterChainDirectory::GetEntry(uint32_t index) const
{
    return this->entries[index];
}

DirectoryEntry * ClusterChainDirectory::FindEntry(const char *name) const
{
    char nameBuf[256];
    for (uint32_t i = 0; i < this->entries.size(); ++i) {
        DirectoryEntry *e = this->entries[i];
        e->GetName(nameBuf);
        if (strcmp(name, nameBuf) == 0) {
            return e;
        }
    }

    /* not found */
    return nullptr;
}

int32_t ClusterChainDirectory::FindEntryIndex(const char *name)
{
    char nameBuf[256];
    for (uint32_t i = 0; i < this->entries.size(); ++i) {
        DirectoryEntry *e = this->entries[i];
        e->GetName(nameBuf);
        if (strcmp(name, nameBuf) == 0) {
            return i;
        }
    }

    /* not found */
    return -1;
}

void ClusterChainDirectory::RemoveEntry(uint32_t index)
{
    this->entries.erase(this->entries.begin() + index);
    uint32_t newCount = this->GetFat32EntryCount();
    if (newCount > 0) {
        this->ChangeSize(newCount);
    } else {
        this->ChangeSize(1); // Empty directory consists of 1 cluster
    }
}

DirectoryEntry * ClusterChainDirectory::AddDirectory(const char *name, Device *device)
{
    this->CheckUniqueName(name);

    Fat *fat = this->chain.GetFat();

    ClusterChain cc(fat, 0);
    cc.SetLength(1);

    time_t now = time(0);

    DirectoryEntry *subde = new DirectoryEntry();
    subde->SetName(name);
    subde->SetIsDir(true);
    subde->SetStartCluster(cc.GetStartCluster());
    subde->SetCreatedTime(now);
    subde->SetLastModifiedTime(now);

    this->AddEntry(subde);
    this->Write(device);

    ClusterChainDirectory subDir;
    subDir.Create(cc);

    // Add `.` entry
    DirectoryEntry *dot = new DirectoryEntry();
    dot->SetName(".");
    dot->SetIsDir(true);
    dot->SetStartCluster(subDir.chain.GetStartCluster());
    dot->SetCreatedTime(now);
    dot->SetLastModifiedTime(now);
    subDir.AddEntry(dot);

    // Add `..` entry
    DirectoryEntry *dotdot = new DirectoryEntry();
    dotdot->SetName("..");
    dotdot->SetIsDir(true);
    dotdot->SetStartCluster(this->chain.GetStartCluster());

    // TODO: copy date/time fields from entry to dotdot;

    subDir.AddEntry(dotdot);
    subDir.Write(device);

    return subde;
}

void ClusterChainDirectory::RemoveDirectory(const char *name, Device *device)
{
    uint32_t index = this->FindEntryIndex(name);
    if (index < 0) {
        std::ostringstream msgStream;
        msgStream << "Couldn't find '" << name << "': No such file or directory.";
        throw std::ios_base::failure(msgStream.str());
    }

    return this->RemoveDirectory(index, device);
}

void ClusterChainDirectory::RemoveDirectory(uint32_t index, Device *device)
{
    DirectoryEntry *e = this->GetEntry(index);
    char nameBuf[256];
    e->GetName(nameBuf);
    if (strcmp(nameBuf, ".") == 0 || strcmp(nameBuf, "..") == 0) {
        return;
    }

    Fat *fat = this->chain.GetFat();

    ClusterChainDirectory subDir = ClusterChainDirectory::GetDirectory(device, fat, e);
    for (size_t i = 0; i < subDir.GetEntries().size(); ++i) {
        DirectoryEntry *subde = subDir.GetEntry(i);
        if (subde->IsDir()) {
            subDir.RemoveDirectory(i, device);
        } else {
            subDir.RemoveFile(i, device);
        }
    }

    ClusterChain cc(this->chain.GetFat(), e->GetStartCluster());
    cc.SetLength(0);

    this->RemoveEntry(index);
    this->Write(device);
}

void ClusterChainDirectory::RemoveFile(const char *name, Device *device)
{
    uint32_t index = this->FindEntryIndex(name);
    if (index < 0) {
        std::ostringstream msgStream;
        msgStream << "Couldn't find '" << name << "': No such file or directory.";
        throw std::runtime_error(msgStream.str());
    }

    this->RemoveFile(index, device);
}

void ClusterChainDirectory::RemoveFile(uint32_t index, Device *device)
{
    DirectoryEntry *e = this->GetEntry(index);
    ClusterChain cc(this->chain.GetFat(), e->GetStartCluster());
    cc.SetLength(0);

    this->RemoveEntry(index);
    this->Write(device);
}

DirectoryEntry * ClusterChainDirectory::AddFile(const char *name, Device *device)
{
    this->CheckUniqueName(name);

    time_t now = time(0);

    DirectoryEntry *e = new DirectoryEntry();
    e->SetName(name);
    e->SetIsDir(false);
    e->SetStartCluster(0);
    e->SetDataLength(0);
    e->SetCreatedTime(now);
    e->SetLastModifiedTime(now);

    this->AddEntry(e);
    this->Write(device);

    return e;
}

ClusterChainFile* ClusterChainDirectory::GetFile(Fat *fat, DirectoryEntry *e)
{
    ClusterChain *cc = new ClusterChain(fat, e->GetStartCluster());
    ClusterChainFile *file = new ClusterChainFile(e, cc);
    return file;
}

ClusterChainDirectory ClusterChainDirectory::GetDirectory(Device *device, Fat *fat, DirectoryEntry *e)
{
    uint32_t firstCluster = e->GetStartCluster();
    ClusterChainDirectory dir;
    dir.Read(device, fat, firstCluster, false);

    return dir;
}

void ClusterChainDirectory::Move(Device *device, DirectoryEntry *e, ClusterChainDirectory& dest, const char *newName)
{
    dest.CheckUniqueName(newName);

    char oldName[256];
    e->GetName(oldName);
    uint32_t index = this->FindEntryIndex(oldName);
    this->RemoveEntry(index);
    this->Write(device);

    e->SetName(newName);
    dest.AddEntry(e);
    dest.Write(device);

    if (e->IsDir()) {
        ClusterChainDirectory dir = GetDirectory(device, dest.chain.GetFat(), e);
        DirectoryEntry *dotdot = dir.FindEntry("..");
        assert(dotdot->GetStartCluster() == this->chain->GetStartCluster());
        dotdot->SetStartCluster(dest.chain.GetStartCluster());

        // Write changes to the disk;
        dir.Write(device);
    }
}

void ClusterChainDirectory::CopyDirectory(Device *device, DirectoryEntry *e, ClusterChainDirectory& dest) const
{
    char nameBuf[256];
    e->GetName(nameBuf);

    this->CopyDirectory(device, e, dest, nameBuf);
}

void ClusterChainDirectory::CopyDirectory(Device *device, DirectoryEntry *e, ClusterChainDirectory& dest, const char *newName) const
{
    assert(e->IsDir());

    ClusterChainDirectory orig = GetDirectory(device, dest.chain.GetFat(), e);

    DirectoryEntry *copye = dest.AddDirectory(newName, device);
    ClusterChainDirectory copy = ClusterChainDirectory::GetDirectory(device, dest.chain.GetFat(), copye);

    char nameBuf[256];
    for (uint32_t i = 0; i < orig.entries.size(); ++i) {
        DirectoryEntry *child = orig.entries[i];
        if (child->IsDir()) {
            child->GetName(nameBuf);
            if (strcmp(nameBuf, ".") != 0 && strcmp(nameBuf, "..") != 0) {
                orig.CopyDirectory(device, child, copy);
            }
        } else {
            orig.CopyFile(device, child, copy);
        }
    }

    copy.Write(device);
}

void ClusterChainDirectory::CopyFile(Device *device, DirectoryEntry *e, ClusterChainDirectory& dest) const
{
    char nameBuf[256];
    e->GetName(nameBuf);

    this->CopyFile(device, e, dest, nameBuf);
}

void ClusterChainDirectory::CopyFile(Device *device, DirectoryEntry *e, ClusterChainDirectory& dest, const char *newName) const
{
    assert(e->IsFile());

    Fat *fat = dest.chain.GetFat();
    DirectoryEntry *copye = dest.AddFile(newName, device);

    // Copy the file content via buffer
    const int bufferSize = 4096;
    uint8_t buf[bufferSize];

    ClusterChainFile *orig = GetFile(fat, e);
    ClusterChainFile *copy = dest.GetFile(fat, copye);

    uint32_t pos = 0;
    uint32_t nread = orig->Read(device, pos, bufferSize, buf);
    while (nread > 0) {
        copy->Write(device, pos, nread, buf);
        pos += nread;
        nread = orig->Read(device, pos, bufferSize, buf);
    }

    // Free memory
    delete orig;
    delete copy;
}

void ClusterChainDirectory::SetName(Device *device, DirectoryEntry *e, const char *name)
{
    this->Move(device, e, *this, name);
}

ClusterChainDirectory::~ClusterChainDirectory()
{
    Cleanup();
}
