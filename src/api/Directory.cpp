#include <queue>
#include "../../include/api/Directory.h"
#include "../../include/Common.h"

using namespace org::vfat;
using namespace org::vfat::api;

//Directory::Directory(FileSystem *fs, DirectoryEntry *e)
//{
//    this->fs = fs;
//    this->entry = e;

//    if (this->IsRoot()) {
//        this->cchDir = fs->GetRootDirectory();
//    } else {
//        this->cchDir = ClusterChainDirectory::GetDirectory(fs->GetDevice(), fs->GetFat(), e);
//    }
//}

Directory::Directory(FileSystem *fs, Path *path)
{
    this->fs = fs;
    this->path = path;

    if (path->IsRoot()) {
        this->parentCchDir = nullptr;
        this->entry = nullptr;
        this->cchDir = fs->GetRootDirectory();
    } else {
        std::queue<ClusterChainDirectory*> subDirectories;
        ClusterChainDirectory *dir = fs->GetRootDirectory();
        DirectoryEntry *e;
        ClusterChainDirectory *parentDir;
        for (size_t i = 0; i < path->GetItemCount(); i++) {
            const char *cname = path->GetItem(i).c_str();
            e = dir->FindEntry(cname);
            if (e == nullptr) {
                throw std::runtime_error("Directory doesn't exist.");
            }

            ClusterChainDirectory *subDir = ClusterChainDirectory::GetDirectory(fs->GetDevice(), fs->GetFat(), e);
            subDirectories.push(subDir);
            parentDir = dir;
            dir = subDir;
        }

        while (subDirectories.size() > 2) {
            ClusterChainDirectory *subDir = subDirectories.front();
            delete subDir;
            subDirectories.pop();
        }

        this->parentCchDir = parentDir;
        this->entry = e;
        this->cchDir = dir;
    }
}

Directory* Directory::GetRoot(FileSystem *fs)
{
    return new Directory(fs, new Path());
}

Directory::~Directory()
{
//    if (this->entry != nullptr) {
//        delete this->entry;
//    }

//    if (!this->path->IsRoot()) {
//        if (this->parentCchDir != nullptr && this->parentCchDir != this->fs->GetRootDirectory()) {
//            delete this->parentCchDir; // delete this->entry;  will be invoked automatically;
//        }

//        delete this->cchDir;
//    }

    if (this->parentCchDir != nullptr && this->parentCchDir != this->fs->GetRootDirectory()) {
        delete this->parentCchDir; // delete this->entry;  will be invoked automatically;
    }

    if (this->cchDir != this->fs->GetRootDirectory()) {
        delete this->cchDir;
    }

    delete this->path;
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
            char nameBuf[256];
            e->GetName(nameBuf);
            Path *dirPath = this->path->Clone();
            dirPath->Combine(nameBuf);
            Directory *dir = new Directory(this->fs, dirPath);
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
            char nameBuf[256];
            e->GetName(nameBuf);
            Path *filePath = this->path->Clone();
            filePath->Combine(nameBuf);
            File *file = new File(this->fs, this->cchDir, e, filePath);
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

Directory* Directory::GetDirectory(std::string path) const
{
//    std::vector<std::string> dirNames;
//    Utils::StringSplit(path, dirNames, '/');

//    ClusterChainDirectory *rootDir = this->fs->GetRootDirectory();
//    FileDisk *device = this->fs->GetDevice();
//    Fat *fat = this->fs->GetFat();

//    ClusterChainDirectory *dir;
//    if (path.at(0) == '/') {
//        dir = rootDir;
//    } else {
//        dir = this->cchDir;
//    }

//    DirectoryEntry *e = nullptr;
//    for (std::string& dirName : dirNames) {
//        e = dir->FindEntry(dirName.c_str());
//        if (e == nullptr) {
//            throw std::runtime_error("Directory doesn't exist.");
//        }

//        //e = e->Clone();
//        ClusterChainDirectory *subDir = ClusterChainDirectory::GetDirectory(device, fat, e);
//        if (dir != rootDir && dir != this->cchDir) {
//            delete dir;
//        }

//        dir = subDir;
//    }

//    if (dir != rootDir && dir != this->cchDir) {
//        delete dir;
//    }

//    Path *pathObj = this->path->Combine(path);

    Path *dirPath = this->path->Clone();
    dirPath->Combine(path);
    return new Directory(this->fs, dirPath);
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

//Directory* Directory::GetDirectory(string name) const
//{
//    DirectoryEntry *e = this->cchDir->FindEntry(name.c_str());
//    if (e == nullptr) {
//        throw std::runtime_error("Directory doesn't exist.");
//    }

//    return new Directory(this->fs, e);
//}

File* Directory::GetFile(string name) const
{
    DirectoryEntry *e = this->cchDir->FindEntry(name.c_str());
    if (e == nullptr) {
        throw std::runtime_error("File doesn't exist.");
    }

    Path *filePath = this->path->Clone();
    filePath->Combine(name);
    return new File(this->fs, this->cchDir, e, filePath);
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
