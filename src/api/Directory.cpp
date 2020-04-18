#include <queue>
#include "../../include/api/Directory.h"
#include "../../include/Common.h"

using namespace org::vfat;
using namespace org::vfat::api;

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
    if (this->parentCchDir != nullptr) {
        delete this->parentCchDir; // delete this->entry;  will be invoked automatically;
    }

    delete this->cchDir;
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
            File *file = new File(this->fs, /*this->cchDir, e,*/ filePath);
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

//File* Directory::GetFile(string name) const
//{
//    DirectoryEntry *e = this->cchDir->FindEntry(name.c_str());
//    if (e == nullptr) {
//        throw std::runtime_error("File doesn't exist.");
//    }

//    Path *filePath = this->path->Clone();
//    filePath->Combine(name);
//    return new File(this->fs, this->cchDir, e, filePath);
//}

File* Directory::GetFile(string path) const
{
    Path *filePath = this->path->Clone();
    filePath->Combine(path);
    return new File(this->fs, filePath);
}

void Directory::DeleteDirectory(string name) const
{
    const char *cname = name.c_str();
    this->cchDir->RemoveDirectory(cname, this->fs->GetDevice());
}

void Directory::DeleteFile(string name) const
{
    const char *cname = name.c_str();
    this->cchDir->RemoveFile(cname, this->fs->GetDevice());
}

void Directory::Write() const
{
    this->cchDir->Write(this->fs->GetDevice());
}

void Directory::Move(string srcPath, string destPath)
{
    Path *srcPathObj = this->path->Clone();
    srcPathObj->Combine(srcPath);

    ClusterChainDirectory *srcDir = this->fs->GetRootDirectory();
    DirectoryEntry *srcEntry = nullptr;
    size_t i;
    for (i = 0; i < srcPathObj->GetItemCount() - 1; i++) {
        const char *cname = srcPathObj->GetItem(i).c_str();
        srcEntry = srcDir->FindEntry(cname);
        if (srcEntry == nullptr) {
            throw std::runtime_error("Directory doesn't exist.");
        }

        ClusterChainDirectory *subDir = ClusterChainDirectory::GetDirectory(fs->GetDevice(), fs->GetFat(), srcEntry);
        delete srcDir;

        srcDir = subDir;
    }

    const char *cname = srcPathObj->GetItem(i).c_str();
    srcEntry = srcDir->FindEntry(cname);
    if (srcEntry == nullptr) {
        throw std::runtime_error("Directory doesn't exist.");
    }

    Path *destPathObj = this->path->Clone();
    destPathObj->Combine(destPath);

    ClusterChainDirectory *destDir = this->fs->GetRootDirectory();
    for (i = 0; i < destPathObj->GetItemCount(); i++) {
        const char *cname = destPathObj->GetItem(i).c_str();
        DirectoryEntry *e = destDir->FindEntry(cname);
        if (e == nullptr) {
            throw std::runtime_error("Directory doesn't exist.");
        }

        ClusterChainDirectory *subDir = ClusterChainDirectory::GetDirectory(fs->GetDevice(), fs->GetFat(), e);
        delete destDir;
        destDir = subDir;
    }

    Path *srcNormalizedPathObj = this->path->Clone();
    srcNormalizedPathObj->Combine(srcPath, true);
    string fileName = srcNormalizedPathObj->GetItem(srcNormalizedPathObj->GetItemCount() - 1);

    srcDir->Move(this->fs->GetDevice(), srcEntry, destDir, fileName.c_str());

    delete srcPathObj;
    delete destPathObj;
    delete srcNormalizedPathObj;

    if (srcDir != this->fs->GetRootDirectory()) {
        delete srcDir;
    }

    if (destDir != this->fs->GetRootDirectory()) {
        delete destDir;
    }
}
