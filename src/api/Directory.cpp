#include "../../include/api/Directory.h"
#include "../../include/Common.h"

using namespace org::vfat;
using namespace org::vfat::api;

Directory::Directory(FileSystem *fs, DirectoryEntry *e)
{
    this->fs = fs;
    this->entry = e;

    if (this->IsRoot()) {
        this->cchDir = fs->GetRootDirectory();
    } else {
        this->cchDir = ClusterChainDirectory::GetDirectory(fs->GetDevice(), fs->GetFat(), e);
    }
}

Directory* Directory::GetRoot(FileSystem *fs)
{
    return new Directory(fs, nullptr);
}

Directory::~Directory()
{
//    if (this->entry != nullptr) {
//        delete this->entry;
//    }

    if (this->cchDir != this->fs->GetRootDirectory()) {
        delete this->cchDir;
    }
}

bool Directory::IsRoot() const
{
    return this->entry == nullptr ||
            (this->entry->GetStartCluster() == this->fs->GetBootSector()->GetRootDirFirstCluster());
}

void Directory::GetDirectories(std::vector<Directory*>& container) const
{
    std::vector<DirectoryEntry *> *entries = this->cchDir->GetEntries();
    for (size_t i = 0; i < entries->size(); i++) {
        DirectoryEntry *e = entries->at(i);
        if (e->IsDir()) {
            Directory *dir = new Directory(this->fs, e);
            container.push_back(dir);
        }
    }
}

void Directory::GetFiles(std::vector<File*>& container) const
{
    std::vector<DirectoryEntry *> *entries = this->cchDir->GetEntries();
    for (size_t i = 0; i < entries->size(); i++) {
        DirectoryEntry *e = entries->at(i);
        if (e->IsFile()) {
            File *file = new File(this->fs, e);
            container.push_back(file);
        }
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
        dir = this->cchDir;
    }

    DirectoryEntry *e = nullptr;
    for (std::string& dirName : dirNames) {
        e = dir->FindEntry(dirName.c_str());
        if (e == nullptr) {
            throw std::runtime_error("Directory doesn't exist.");
        }

        //e = e->Clone();
        ClusterChainDirectory *subDir = ClusterChainDirectory::GetDirectory(device, fat, e);
        if (dir != rootDir && dir != this->cchDir) {
            delete dir;
        }

        dir = subDir;
    }

//    if (dir != rootDir && dir != this->cchDir) {
//        delete dir;
//    }

    return new Directory(this->fs, e);
}

void Directory::CreateDirectory(std::string name) const
{
    const char *cname = name.c_str();
    this->cchDir->AddDirectory(cname, this->fs->GetDevice());
}

void Directory::CreateFile(std::string name) const
{
    const char *cname = name.c_str();
    this->cchDir->AddFile(cname, this->fs->GetDevice());
}

Directory* Directory::GetDirectory(string name) const
{
    DirectoryEntry *e = this->cchDir->FindEntry(name.c_str());
    if (e == nullptr) {
        throw std::runtime_error("Directory doesn't exist.");
    }

    return new Directory(this->fs, e);
}

File* Directory::GetFile(string name) const
{
    DirectoryEntry *e = this->cchDir->FindEntry(name.c_str());
    if (e == nullptr) {
        throw std::runtime_error("Directory doesn't exist.");
    }

    return new File(this->fs, e);
}

void Directory::DeleteDirectory(string name) const
{
    const char *cname = name.c_str();
    this->cchDir->RemoveDirectory(cname);
}

void Directory::DeleteFile(string name) const
{
    const char *cname = name.c_str();
    this->cchDir->RemoveFile(cname);
}

void Directory::Write() const
{
    this->cchDir->Write(this->fs->GetDevice());
}
