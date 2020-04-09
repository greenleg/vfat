#include "../../include/api/Directory.h"
#include "../../include/Common.h"

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

bool Directory::IsRoot() const
{
    return this->entry == nullptr ||
            (this->entry->GetStartCluster() == this->fs->GetBootSector()->GetRootDirFirstCluster());
}

void Directory::GetDirectories(std::vector<Directory*>& container) const
{
    ClusterChainDirectory *cchDir;    
    if (IsRoot()) {
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

    if (!IsRoot()) {
        delete cchDir;
    }
}

string Directory::GetName() const
{
    char nameBuf[256];
    if (this->entry == nullptr) {
        return "/";
    }

    this->entry->GetName(nameBuf);
    string s(nameBuf);
    return s; // return a copy of the local variable s;
}

Directory* Directory::ChangeDirectory(std::string path) const
{
    std::vector<std::string> dirNames;
    Utils::StringSplit(path, dirNames, '/');

    ClusterChainDirectory *rootDir = this->fs->GetRootDirectory();
    FileDisk *device = this->fs->GetDevice();
    Fat *fat = this->fs->GetFat();

    ClusterChainDirectory *dir;
    if (path.at(0) == '/') {
        dir = rootDir;
    } else {
        dir = ClusterChainDirectory::GetDirectory(device, fat, this->entry);
    }

    DirectoryEntry *e = nullptr;

    for (std::string& dirName : dirNames) {
        e = dir->FindEntry(dirName.c_str());
        if (e == nullptr) {
            throw std::runtime_error("Directory doesn't exist.");
        }

        e = e->Clone();
        ClusterChainDirectory *subDir = ClusterChainDirectory::GetDirectory(device, fat, e);
        if (dir != rootDir) {
            delete dir;
        }

        dir = subDir;
    }

    if (dir != rootDir) {
        delete dir;
    }

    return (e != nullptr) ? new Directory(this->fs, e) : new Directory(this->fs);
}

void Directory::CreateDirectory(std::string name) const
{
    FileDisk *device = this->fs->GetDevice();
    Fat *fat = this->fs->GetFat();

    ClusterChainDirectory *dir;
    if (IsRoot()) {
        dir = this->fs->GetRootDirectory();
    } else {
        dir = ClusterChainDirectory::GetDirectory(device, fat, this->entry);
    }

    const char *cname = name.c_str();
    dir->AddDirectory(cname, device);

    if (dir != this->fs->GetRootDirectory()) {
        delete dir;
    }
}

Directory* Directory::GetDirectory(string name) const
{
    ClusterChainDirectory *dir;
    if (IsRoot()) {
        dir = this->fs->GetRootDirectory();
    } else {
        dir = ClusterChainDirectory::GetDirectory(this->fs->GetDevice(), this->fs->GetFat(), this->entry);
    }

    DirectoryEntry *e = dir->FindEntry(name.c_str());
    if (e == nullptr) {
        throw std::runtime_error("Directory doesn't exist.");
    }

    if (dir != this->fs->GetRootDirectory()) {
        delete dir;
    }

    return new Directory(this->fs, e->Clone());
}
