#include "../../include/api/Directory.h"

using namespace org::vfat;
using namespace org::vfat::api;

Directory::Directory(FileSystem *fs, DirectoryEntry *e)
{
    this->fs = fs;
    this->entry = e;
}

Directory::Directory(FileSystem *fs)
{
    this->fs = fs;
    this->entry = nullptr;
}

Directory* Directory::GetRoot(FileSystem *fs)
{
    return new Directory(fs);
}

Directory::~Directory()
{
    if (this->entry != nullptr) {
        delete this->entry;
    }
}

void Directory::GetDirectories(std::vector<Directory*>& container) const
{
    ClusterChainDirectory *cchDir;
    if (this->entry == nullptr) {
        cchDir = this->fs->GetRootDirectory();
    } else {
        FileDisk *dev = this->fs->GetDevice();
        Fat *fat = this->fs->GetFat();
        cchDir = ClusterChainDirectory::GetDirectory(dev, fat, this->entry);
    }

    std::vector<DirectoryEntry *> *entries = cchDir->GetEntries();
    for (int i = 0; i < entries->size(); i++) {
        DirectoryEntry *e = entries->at(i);
        DirectoryEntry *copye = e->Clone();
        Directory *dir = new Directory(this->fs, copye);
        container.push_back(dir);
    }

    delete cchDir;
}

string Directory::GetName() const
{
    char nameBuf[256];
    this->entry->GetName(nameBuf);
    string s(nameBuf);
    return s; // return a copy of the local variable s;
}
