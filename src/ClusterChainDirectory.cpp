#include <assert.h>
#include <stdexcept>

#include "../include/string.h"
#include "../include/Fat.h"
#include "../include/ClusterChainDirectory.h"
#include "../include/ClusterChainFile.h"
#include "../include/DirectoryEntry.h"
#include "../include/BootSector.h"

using namespace org::vfat;

void ClusterChainDirectory::CheckUniqueName(const char *name)
{
    if (this->FindEntry(name) != nullptr) {
        throw std::runtime_error("Directory already exists.");
    }
}

//static void cchdir_read_entries(struct cchdir *dir, uint8_t *buffer)
//{
//    uint32_t offset = 0;
//    uint8_t entry_type;
//    DirectoryEntry e;
//    uint32_t i;

//    for (i = 0; i < dir->capacity; ++i) {
//        entry_type = buffer[offset];
//        if (entry_type == NO_DIR_ENTRY) {
//            break;
//        }

//        e.Read(buffer + offset);
//        alist_add(dir->entries, &e);
//        offset += e.GetEntryCount() * FAT_DIR_ENTRY_SIZE;
//    }
//}

void ClusterChainDirectory::ReadEntries(uint8_t *buffer)
{
    uint32_t offset = 0;
    for (uint32_t i = 0; i < this->capacity; i++) {
        uint8_t entryType = buffer[offset];
        if (entryType == NO_DIR_ENTRY) {
            break;
        }

        DirectoryEntry *e = new DirectoryEntry();
        e->Read(buffer + offset);
        this->entries->push_back(e);

        offset += e->GetFat32EntryCount() * FAT_DIR_ENTRY_SIZE;
    }
}

//static uint32_t cchdir_write_entries(struct cchdir* dir, uint8_t *buffer, uint32_t bufsize)
//{
//    uint32_t offset = 0;
//    uint32_t i;

//    DirectoryEntry e;
//    for (i = 0; i < alist_count(dir->entries); ++i) {
//        alist_get(dir->entries, i, &e);
//        e.Write(buffer + offset);
//        offset += e.GetEntryCount() * FAT_DIR_ENTRY_SIZE;
//    }

//    if (offset < bufsize) {
//        // Write the end-of-list marker.
//        buffer[offset] = NO_DIR_ENTRY;
//        offset += 1;
//    }

//    // offset is equal to number of the actually written bytes.
//    return offset;
//}

uint32_t ClusterChainDirectory::WriteEntries(uint8_t *buffer, uint32_t bufferSize) const
{
    uint32_t offset = 0;
    for (uint32_t i = 0; i < this->entries->size(); i++) {
        DirectoryEntry *e = this->entries->at(i);
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

//void cchdir_formatdev(/*in*/ org::vfat::FileDisk *device,
//                      /*in*/ uint64_t vol_size,
//                      /*in*/ uint16_t bytes_per_sect,
//                      /*in*/ uint16_t sect_per_clus)
//{
//    BootSector bootSector;
//    bootSector.Create(vol_size, bytes_per_sect, sect_per_clus);
//    bootSector.Write(device);

//    Fat fat(&bootSector);
//    fat.Create();

//    struct cchdir root;
//    cchdir_createroot(&fat, &root);

//    this->Write(&root, device);
//    fat.Write(device);

//    cchdir_destruct(&root);
//    //fat_destruct(&fat); will be invoked automatically
//}

void ClusterChainDirectory::FormatDevice(FileDisk * device, uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorPerCluster)
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

//void cchdir_readdir(/*in*/ org::vfat::FileDisk *device,
//                    /*in*/ Fat *fat,
//                    /*in*/ uint32_t first_cluster,
//                    /*in*/ bool root,
//                    /*out*/ struct cchdir* dir)
//{
//    ClusterChain *cc = new ClusterChain(fat, first_cluster);

//    uint64_t size = cc->GetSizeInBytes();
//    uint8_t buffer[size];
//    cc->ReadData(device, 0, size, buffer);

//    dir->chain = cc;
//    dir->root = root;
//    dir->capacity = size / FAT_DIR_ENTRY_SIZE;
//    dir->entries = static_cast<struct alist *>(malloc(sizeof(struct alist)));
//    alist_create(dir->entries, sizeof(DirectoryEntry));

//    cchdir_read_entries(dir, buffer);
//}

void ClusterChainDirectory::Read(FileDisk *device, Fat *fat, uint32_t firstCluster, bool isRoot)
{
    ClusterChain *cc = new ClusterChain(fat, firstCluster);

    uint64_t size = cc->GetSizeInBytes();
    uint8_t buffer[size];
    cc->ReadData(device, 0, size, buffer);

    this->chain = cc;
    this->isRoot = isRoot;
    this->capacity = size / FAT_DIR_ENTRY_SIZE;
    this->entries = new std::vector<DirectoryEntry *>();
    this->ReadEntries(buffer);
}

//void cchdir_write(struct cchdir* dir, org::vfat::FileDisk *device)
//{
//    uint32_t nbytes = dir->capacity * FAT_DIR_ENTRY_SIZE;
//    uint8_t buf[nbytes];
//    uint32_t realbytes = cchdir_write_entries(dir, buf, nbytes);
//    dir->chain->WriteData(device, 0, realbytes, buf);
//}

void ClusterChainDirectory::Write(FileDisk *device) const
{
    uint32_t nbytes = this->capacity * FAT_DIR_ENTRY_SIZE;
    uint8_t buffer[nbytes];
    uint32_t realBytes = this->WriteEntries(buffer, nbytes);
    this->chain->WriteData(device, 0, realBytes, buffer);
}

//void cchdir_create(ClusterChain *cc, struct cchdir *dir)
//{
//    dir->chain = cc;
//    dir->root = false;
//    dir->capacity = cc->GetSizeInBytes() / FAT_DIR_ENTRY_SIZE;
//    dir->entries = static_cast<struct alist *>(malloc(sizeof(struct alist)));
//    alist_create(dir->entries, sizeof(DirectoryEntry));
//}

void ClusterChainDirectory::Create(ClusterChain *cc)
{
    this->chain = cc;
    this->isRoot = false;
    this->capacity = cc->GetSizeInBytes() / FAT_DIR_ENTRY_SIZE;
    this->entries = new std::vector<DirectoryEntry *>();
}

void ClusterChainDirectory::CreateRoot(Fat *fat)
{
    BootSector *bootSector = fat->GetBootSector();
    ClusterChain *cc = new ClusterChain(fat, 0);
    cc->SetLength(1);
    bootSector->SetRootDirFirstCluster(cc->GetStartCluster());

    this->chain = cc;
    this->isRoot = true;
    this->capacity = cc->GetSizeInBytes() / FAT_DIR_ENTRY_SIZE;
//    dir->entries = static_cast<struct alist *>(malloc(sizeof(struct alist)));
//    alist_create(dir->entries, sizeof(DirectoryEntry));
    this->entries = new std::vector<DirectoryEntry *>();
}

//void cchdir_readroot(/*in*/ org::vfat::FileDisk *device, /*in*/ Fat *fat, /*out*/ struct cchdir *dir)
//{
//    cchdir_readdir(device, fat, fat->GetBootSector()->GetRootDirFirstCluster(), true, dir);
//}

void ClusterChainDirectory::ReadRoot(FileDisk *device, Fat *fat)
{
    this->Read(device, fat, fat->GetBootSector()->GetRootDirFirstCluster(), true);
}

//void cchdir_changesize(struct cchdir *dir, uint32_t fat32_entry_cnt)
//{
//    uint32_t size = fat32_entry_cnt * FAT_DIR_ENTRY_SIZE;
//    uint32_t new_size = dir->chain->SetSizeInBytes(size);
//    dir->capacity = new_size / FAT_DIR_ENTRY_SIZE;
//}

void ClusterChainDirectory::ChangeSize(uint32_t fat32EntryCount)
{
    uint32_t size = fat32EntryCount * FAT_DIR_ENTRY_SIZE;
    uint32_t newSize = this->chain->SetSizeInBytes(size);
    this->capacity = newSize / FAT_DIR_ENTRY_SIZE;
}

uint32_t ClusterChainDirectory::GetFat32EntryCount() const
{
    uint32_t n = 0;
    for (uint32_t i = 0; i < this->entries->size(); i++) {
        DirectoryEntry *e = this->entries->at(i);
        n += e->GetFat32EntryCount();
    }

    return n;
}

//void cchdir_addentry(struct cchdir *dir, DirectoryEntry *e)
//{
//    uint32_t new_cnt = get_fat32_entry_cnt(dir);
//    new_cnt += e->GetEntryCount();

//    if (new_cnt > dir->capacity) {
//        cchdir_changesize(dir, new_cnt);
//    }

//    alist_add(dir->entries, e);
//}

void ClusterChainDirectory::AddEntry(DirectoryEntry *e)
{
    uint32_t newCount = this->GetFat32EntryCount() + e->GetFat32EntryCount();
    if (newCount > this->capacity) {
        this->ChangeSize(newCount);
    }

    this->entries->push_back(e);
}

DirectoryEntry * ClusterChainDirectory::GetEntry(uint32_t index) const
{
    return this->entries->at(index);
}

//bool cchdir_findentry(/*in*/ struct cchdir *dir, /*in*/ const char *name, /*out*/ DirectoryEntry *e)
//{
//    uint32_t i;
//    char namebuf[256];

//    for (i = 0; i < alist_count(dir->entries); ++i) {
//        alist_get(dir->entries, i, e);
//        e->GetName(namebuf);
//        if (strcmp(name, namebuf) == 0) {
//            return true;
//        }
//    }

//    /* not found */
//    return false;
//}

DirectoryEntry * ClusterChainDirectory::FindEntry(const char *name) const
{
    char nameBuf[256];
    for (uint32_t i = 0; i < this->entries->size(); i++) {
        DirectoryEntry *e = this->entries->at(i);
        e->GetName(nameBuf);
        if (strcmp(name, nameBuf) == 0) {
            return e;
        }
    }

    /* not found */
    return nullptr;
}

//bool cchdir_findentryidx(/*in*/ struct cchdir *dir, /*in*/ const char *name, /*out*/ uint32_t *idx)
//{
//    uint32_t i;
//    char namebuf[256];
//    DirectoryEntry e;

//    for (i = 0; i < alist_count(dir->entries); ++i) {
//        alist_get(dir->entries, i, &e);
//        e.GetName(namebuf);
//        if (strcmp(name, namebuf) == 0) {
//            *idx = i;
//            return true;
//        }
//    }

//    /* not found */
//    return false;
//}

int32_t ClusterChainDirectory::FindEntryIndex(const char *name)
{
    char nameBuf[256];
    for (uint32_t i = 0; i < this->entries->size(); i++) {
        DirectoryEntry *e = this->entries->at(i);
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
    this->entries->erase(this->entries->begin() + index);
    uint32_t newCount = this->GetFat32EntryCount();
    if (newCount > 0) {
        this->ChangeSize(newCount);
    } else {
        this->ChangeSize(1); // Empty directory consists of 1 cluster
    }
}

//bool cchdir_createsubdir(/*in*/ struct cchdir *parentdir, /*out*/ struct cchdir *subdir, /*out*/ DirectoryEntry* subde)
//{
//    DirectoryEntry dot;
//    DirectoryEntry dotdot;
//    Fat *fat = parentdir->chain->GetFat();

////    struct cch *cc = static_cast<struct cch *>(malloc(sizeof(struct cch)));
////    if (!cch_create(cc, fat, 1)) {
////        return false;
////    }

//    ClusterChain *cc = new ClusterChain(fat, 0);
//    cc->SetLength(1);

//    //lfnde_create(subde);
//    subde->SetIsDir(true);
//    subde->SetStartCluster(cc->GetStartCluster());

//    cchdir_create(cc, subdir);

//    /* Add "." entry */
//    //lfnde_create(&dot);
//    dot.SetName(".");
//    dot.SetIsDir(true);
//    dot.SetStartCluster(subdir->chain->GetStartCluster());
//    // TODO: copy date/time fields from entry to dot;
//    cchdir_addentry(subdir, &dot);

//    /* Add ".." entry */
//    //lfnde_create(&dotdot);
//    dotdot.SetName("..");
//    dotdot.SetIsDir(true);
//    dotdot.SetStartCluster(parentdir->chain->GetStartCluster());
//    // TODO: copy date/time fields from entry to dotdot;
//    cchdir_addentry(subdir, &dotdot);

//    //cchdir_write(subdir, disk);
//    return true;
//}

//DirectoryEntry * ClusterChainDirectory::CreateSubDirectory()
//{
//    Fat *fat = this->chain->GetFat();

//    ClusterChain *cc = new ClusterChain(fat, 0);
//    cc->SetLength(1);

//    DirectoryEntry *subde = new DirectoryEntry();
//    subde->SetIsDir(true);
//    subde->SetStartCluster(cc->GetStartCluster());

//    ClusterChainDirectory *subDir = new ClusterChainDirectory();
//    subDir->Create(cc);

//    // Add `.` entry
//    DirectoryEntry *dot = new DirectoryEntry();
//    dot->SetName(".");
//    dot->SetIsDir(true);
//    dot->SetStartCluster(subDir->chain->GetStartCluster());
//    // TODO: copy date/time fields from entry to dot;
//    subDir->AddEntry(dot);

//    // Add `..` entry
//    DirectoryEntry *dotdot = new DirectoryEntry();
//    dotdot->SetName("..");
//    dotdot->SetIsDir(true);
//    dotdot->SetStartCluster(this->chain->GetStartCluster());
//    // TODO: copy date/time fields from entry to dotdot;
//    subDir->AddEntry(dotdot);

//    //cchdir_write(subdir, disk);
//    return subde;
//}

//bool cchdir_adddir(/*in*/ struct cchdir *dir,
//                   /*in*/ const char *name,
//                   /*out*/ DirectoryEntry *subde,
//                   /*out*/ struct cchdir *subdir)
//{
//    if (!check_unique_name(dir, name)) {
//        return false;
//    }

//    if (!cchdir_createsubdir(dir, subdir, subde)) {
//        return false;
//    }

//    subde->SetName(name);
//    cchdir_addentry(dir, subde);

//    return true;
//}

DirectoryEntry * ClusterChainDirectory::AddDirectory(const char *name, FileDisk *device)
{
    this->CheckUniqueName(name);

    Fat *fat = this->chain->GetFat();

    ClusterChain *cc = new ClusterChain(fat, 0);
    cc->SetLength(1);

    DirectoryEntry *subde = new DirectoryEntry();
    subde->SetName(name);
    subde->SetIsDir(true);
    subde->SetStartCluster(cc->GetStartCluster());
    this->AddEntry(subde);
    this->Write(device);

    ClusterChainDirectory *subDir = new ClusterChainDirectory();
    subDir->Create(cc);

    // Add `.` entry
    DirectoryEntry *dot = new DirectoryEntry();
    dot->SetName(".");
    dot->SetIsDir(true);
    dot->SetStartCluster(subDir->chain->GetStartCluster());
    // TODO: copy date/time fields from entry to dot;
    subDir->AddEntry(dot);

    // Add `..` entry
    DirectoryEntry *dotdot = new DirectoryEntry();
    dotdot->SetName("..");
    dotdot->SetIsDir(true);
    dotdot->SetStartCluster(this->chain->GetStartCluster());
    // TODO: copy date/time fields from entry to dotdot;
    subDir->AddEntry(dotdot);

    subDir->Write(device);

    return subde;
}

//bool cchdir_removedir(/*in*/ struct cchdir *dir, /*in*/ const char *name)
//{
//    DirectoryEntry e;
//    uint32_t idx;

//    if (!cchdir_findentryidx(dir, name, &idx)) {
//        return false;
//    }

//    cchdir_getentry(dir, idx, &e);

////    cc.fat = dir->chain->fat;
////    cc.start_cluster = e.sede->first_cluster;
////    if (!cch_setlen(&cc, 0)) {
////        return false;
////    }
//    ClusterChain cc(dir->chain->GetFat(), e.GetStartCluster());
//    cc.SetLength(0);

//    cchdir_removeentry(dir, idx);
//    return true;
//}

void ClusterChainDirectory::RemoveDirectory(const char *name, FileDisk *device)
{
    uint32_t index = this->FindEntryIndex(name);
    if (index < 0) {
        throw std::runtime_error("Directory doesn't exist.");
    }

    return this->RemoveDirectory(index, device);
}

void ClusterChainDirectory::RemoveDirectory(uint32_t index, FileDisk *device)
{
    DirectoryEntry *e = this->GetEntry(index);
    char nameBuf[256];
    e->GetName(nameBuf);
    if (strcmp(nameBuf, ".") == 0 || strcmp(nameBuf, "..") == 0) {
        return;
    }

    Fat *fat = this->chain->GetFat();

    ClusterChainDirectory *subDir = ClusterChainDirectory::GetDirectory(device, fat, e);
    for (size_t i = 0; i < subDir->GetEntries()->size(); i++) {
        DirectoryEntry * subde = subDir->GetEntry(i);
        if (subde->IsDir()) {
            subDir->RemoveDirectory(i, device);
        } else {
            subDir->RemoveFile(i, device);
        }
    }

    delete subDir;

    ClusterChain cc(this->chain->GetFat(), e->GetStartCluster());
    cc.SetLength(0);

    this->RemoveEntry(index);
    this->Write(device);
}

void ClusterChainDirectory::RemoveFile(const char *name, FileDisk *device)
{
    uint32_t index = this->FindEntryIndex(name);
    if (index < 0) {
        throw std::runtime_error("File doesn't exist.");
    }

    this->RemoveFile(index, device);
}

void ClusterChainDirectory::RemoveFile(uint32_t index, FileDisk *device)
{
    DirectoryEntry *e = this->GetEntry(index);
    ClusterChain cc(this->chain->GetFat(), e->GetStartCluster());
    cc.SetLength(0);

    this->RemoveEntry(index);
    this->Write(device);
}

//bool cchdir_addfile(/*in*/ struct cchdir *dir, /*in*/ const char *name, /*out*/ DirectoryEntry *e)
//{
//    if (!check_unique_name(dir, name)) {
//        return false;
//    }

//    //lfnde_create(e);
//    e->SetName(name);
//    e->SetIsDir(false);
//    e->SetStartCluster(0);
//    e->SetDataLength(0);
//    cchdir_addentry(dir, e);

//    return true;
//}

DirectoryEntry * ClusterChainDirectory::AddFile(const char *name, FileDisk *device)
{
    this->CheckUniqueName(name);

    DirectoryEntry *e = new DirectoryEntry();
    e->SetName(name);
    e->SetIsDir(false);
    e->SetStartCluster(0);
    e->SetDataLength(0);
    this->AddEntry(e);
    this->Write(device);

    return e;
}

//void cchdir_getfile(/*in*/ struct cchdir *dir,
//                    /*in*/ DirectoryEntry *e,
//                    /*out*/ struct cchfile *file)
//{
////    struct cch *cc = static_cast<struct cch *>(malloc(sizeof(struct cch)));
////    cc->fat = dir->chain->fat;
////    cc->start_cluster = lfnde_getstartcluster(e);
//    ClusterChain *cc = new ClusterChain(dir->chain->GetFat(), e->GetStartCluster());

//    file->chain = cc;
//    file->entry = e;
//}

ClusterChainFile* ClusterChainDirectory::GetFile(Fat *fat, DirectoryEntry *e)
{
    ClusterChain *cc = new ClusterChain(fat, e->GetStartCluster());
    ClusterChainFile *file = new ClusterChainFile(e, cc);
    return file;
}

//bool cchdir_getdir(/*in*/ org::vfat::FileDisk *device,
//                   /*in*/ Fat *fat,
//                   /*in*/ DirectoryEntry *e,
//                   /*out*/ struct cchdir *dir)
//{
//    uint32_t first_cluster = e->GetStartCluster();
//    cchdir_readdir(device, fat, first_cluster, false, dir);

//    return true;
//}

ClusterChainDirectory* ClusterChainDirectory::GetDirectory(FileDisk *device, Fat *fat, DirectoryEntry *e)
{
    uint32_t firstCluster = e->GetStartCluster();
    ClusterChainDirectory *dir = new ClusterChainDirectory();
    dir->Read(device, fat, firstCluster, false);

    return dir;
}

//bool cchdir_move(/*in*/ org::vfat::FileDisk *device,
//                 /*in*/ struct cchdir *src,
//                 /*in*/ struct DirectoryEntry *e,
//                 /*in*/ struct cchdir *dst,
//                 /*in*/ const char *new_name)
//{
//    if (!check_unique_name(dst, new_name)) {
//        return false;
//    }

//    uint32_t idx;
//    char old_name[256];
//    e->GetName(old_name);
//    cchdir_findentryidx(src, old_name, &idx);
//    cchdir_removeentry(src, idx);
//    e->SetName(new_name);
//    cchdir_addentry(dst, e);

//    if (e->IsDir()) {
//        struct cchdir dir;
//        cchdir_getdir(device, src->chain->GetFat(), e, &dir);

//        DirectoryEntry dotdot;
//        cchdir_findentry(&dir, "..", &dotdot);
//        assert(dotdot.GetStartCluster() == src->chain->GetStartCluster());
//        dotdot.SetStartCluster(dst->chain->GetStartCluster());

//        // Write dir to the disk
//        cchdir_write(&dir, device);
//        cchdir_destruct(&dir);
//    }

//    return true;
//}

void ClusterChainDirectory::Move(FileDisk *device, DirectoryEntry *e, ClusterChainDirectory *dest, const char *newName)
{
    dest->CheckUniqueName(newName);

    char oldName[256];
    e->GetName(oldName);
    uint32_t index = this->FindEntryIndex(oldName);
    this->RemoveEntry(index);
    this->Write(device);

    e->SetName(newName);
    dest->AddEntry(e);
    dest->Write(device);

    if (e->IsDir()) {
        ClusterChainDirectory *dir = GetDirectory(device, dest->chain->GetFat(), e);
        DirectoryEntry *dotdot = dir->FindEntry("..");
        assert(dotdot->GetStartCluster() == this->chain->GetStartCluster());
        dotdot->SetStartCluster(dest->chain->GetStartCluster());

        // Write changes to the disk
        dir->Write(device);
        delete dir;
    }
}

//bool cchdir_copyfile(/*in*/ org::vfat::FileDisk *device,
//                     /*in*/ struct cchdir *src,
//                     /*in*/ DirectoryEntry *e,
//                     /*in*/ struct cchdir *dst)
//{
//    char namebuf[256];
//    e->GetName(namebuf);

//    DirectoryEntry copye;
//    if (!cchdir_addfile(dst, namebuf, &copye)) {
//        return false;
//    }

//    // Copy the file content
//    const int nbytes = 4096;
//    uint8_t buf[nbytes];

//    struct cchfile orig;
//    cchdir_getfile(src, e, &orig);

//    struct cchfile copy;
//    cchdir_getfile(src, &copye, &copy);

//    uint32_t pos = 0;
//    uint32_t nread;
//    cchfile_read(device, &orig, pos, nbytes, &nread, buf);
//    while (nread > 0) {
//        cchfile_write(device, &copy, pos, nread, buf);
//        pos += nread;
//        cchfile_read(device, &orig, pos, nbytes, &nread, buf);
//    }

//    // Free memory
//    cchfile_destruct(&orig);
//    cchfile_destruct(&copy);

//    return true;
//}

//bool cchdir_copydir(/*in*/ org::vfat::FileDisk *device,
//                    /*in*/ struct cchdir *src,
//                    /*in*/ DirectoryEntry *e,
//                    /*in*/ struct cchdir *dst)
//{
//    Fat *fat = src->chain->GetFat();

//    char namebuf[256];
//    e->GetName(namebuf);

//    struct cchdir orig;
//    if (!cchdir_getdir(device, fat, e, &orig)) {
//        return false;
//    }

//    DirectoryEntry copye;
//    struct cchdir copy;
//    if (!cchdir_adddir(dst, namebuf, &copye, &copy)) {
//        return false;
//    }

//    DirectoryEntry child;
//    uint32_t i;
//    for (i = 0; i < alist_count(orig.entries); ++i) {
//        cchdir_getentry(&orig, i, &child);
//        if (child.IsDir()) {
//            child.GetName(namebuf);
//            if (strcmp(namebuf, ".") == 0 || strcmp(namebuf, "..") == 0) {
//                continue;
//            }

//            if (!cchdir_copydir(device, &orig, &child, &copy)) {
//                return false;
//            }
//        } else {
//            if (!cchdir_copyfile(device, &orig, &child, &copy)) {
//                return false;
//            }
//        }
//    }

//    cchdir_write(&copy, device);

//    cchdir_destruct(&orig);
//    cchdir_destruct(&copy);

//    return true;
//}


void ClusterChainDirectory::CopyDirectory(FileDisk *device, DirectoryEntry *e, ClusterChainDirectory *dest) const
{
    assert(e->IsDir());

    char nameBuf[256];
    e->GetName(nameBuf);

    ClusterChainDirectory *orig = GetDirectory(device, dest->chain->GetFat(), e);

    DirectoryEntry *copye = dest->AddDirectory(nameBuf, device);
    ClusterChainDirectory *copy = ClusterChainDirectory::GetDirectory(device, dest->chain->GetFat(), copye);

    for (uint32_t i = 0; i < orig->entries->size(); i++) {
        DirectoryEntry *child = orig->entries->at(i);
        if (child->IsDir()) {
            child->GetName(nameBuf);
            if (strcmp(nameBuf, ".") != 0 && strcmp(nameBuf, "..") != 0) {
                orig->CopyDirectory(device, child, copy);
            }
        } else {
            orig->CopyFile(device, child, copy);
        }
    }

    copy->Write(device);

    delete orig;
    delete copy;
}

void ClusterChainDirectory::CopyFile(FileDisk *device, DirectoryEntry *e, ClusterChainDirectory *dest) const
{
    assert(e->IsFile());

    char nameBuf[256];
    e->GetName(nameBuf);

    Fat *fat = dest->chain->GetFat();
    DirectoryEntry *copye = dest->AddFile(nameBuf, device);

    // Copy the file content via buffer
    const int bufferSize = 4096;
    uint8_t buf[bufferSize];

    ClusterChainFile *orig = this->GetFile(fat, e);
    ClusterChainFile *copy = dest->GetFile(fat, copye);

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

//bool cchdir_setname(/*in*/ org::vfat::FileDisk *device,
//                    /*in*/ struct cchdir *dir,
//                    /*in*/ DirectoryEntry *e,
//                    /*in*/ const char *name)
//{
//    return cchdir_move(device, dir, e, dir, name);
//}

void ClusterChainDirectory::SetName(FileDisk *device, DirectoryEntry *e, const char *name)
{
    this->Move(device, e, this, name);
}

//void cchdir_destruct(/*in*/ struct cchdir *dir)
//{
//    uint32_t i;

//    for (i = 0; i < alist_count(dir->entries); ++i) {
//        DirectoryEntry e;
//        alist_get(dir->entries, i, &e);
//        //lfnde_destruct(&e);
//    }

//    alist_destruct(dir->entries);
//    free(dir->entries);
//    free(dir->chain);
//}

ClusterChainDirectory::~ClusterChainDirectory()
{
    for (uint32_t i = 0; i < this->entries->size(); i++) {
        DirectoryEntry *e = this->entries->at(i);
        delete e;
    }

    delete this->entries;
    delete this->chain;
}
