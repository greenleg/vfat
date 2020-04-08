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

void Directory::GetDirectories(std::vector<Directory*>& container) const
{
    ClusterChainDirectory *cchDir;
    bool pointToRoot = this->entry == nullptr || (this->entry->GetStartCluster() == this->fs->GetBootSector()->GetRootDirFirstCluster());
    if (pointToRoot) {
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

    if (!pointToRoot) {
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

    for (std::string& dirName : dirNames) {
        DirectoryEntry *e = dir->FindEntry(dirName.c_str());
        if (e == nullptr) {
            throw std::runtime_error("Directory doesn't exist.");
        }

        ClusterChainDirectory *subDir = ClusterChainDirectory::GetDirectory(device, fat, e);
        if (dir != rootDir) {
            delete dir;
        }

        dir = subDir;
    }

    //return dir;
}

void Directory::CreateDirectory(std::string name) const
{
    FileDisk *device = this->fs->GetDevice();
    Fat *fat = this->fs->GetFat();

    ClusterChainDirectory *dir;
    if (this->entry == nullptr) {
        dir = this->fs->GetRootDirectory();
    } else {
        dir = ClusterChainDirectory::GetDirectory(device, fat, this->entry);
    }

    const char *cname = name.c_str();
    dir->AddDirectory(cname, device);

    if (this->entry != nullptr) {
        delete dir;
    }
}
