#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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

        time_t now = time(0);

        // Fake root entry;
        this->entry = new DirectoryEntry();
        this->entry->SetStartCluster(fs->GetBootSector()->GetRootDirFirstCluster());
        this->entry->SetCreatedTime(now);
        this->entry->SetLastModifiedTime(now);
        this->entry->SetIsDir(true);
    } else {
//        std::queue<ClusterChainDirectory*> subDirectories;
//        ClusterChainDirectory *dir = fs->GetRootDirectory();
//        DirectoryEntry *e;
//        ClusterChainDirectory *parentDir;
//        for (size_t i = 0; i < path->GetItemCount(); i++) {
//            string name = path->GetItem(i);
//            e = dir->FindEntry(name.c_str());
//            if (e == nullptr) {
//                std::ostringstream msgStream;
//                msgStream << "Couldn't find '" << path->ToString(false) << "': No such file or directory.";
//                throw std::runtime_error(msgStream.str());
//            }

//            ClusterChainDirectory *subDir = ClusterChainDirectory::GetDirectory(fs->GetDevice(), fs->GetFat(), e);
//            subDirectories.push(subDir);
//            parentDir = dir;
//            dir = subDir;
//        }

//        while (subDirectories.size() > 2) {
//            ClusterChainDirectory *subDir = subDirectories.front();
//            delete subDir;
//            subDirectories.pop();
//        }

        ClusterChainDirectory *dir = fs->GetRootDirectory();
        DirectoryEntry *e;
        size_t i = 0;
        for (; i < path->GetItemCount() - 1; i++) {
            string name = path->GetItem(i);
            e = dir->FindEntry(name.c_str());
            if (e == nullptr) {
                std::ostringstream msgStream;
                msgStream << "Couldn't find '" << path->ToString(false) << "': No such file or directory.";
                throw std::runtime_error(msgStream.str());
            }

            ClusterChainDirectory *subDir = ClusterChainDirectory::GetDirectory(fs->GetDevice(), fs->GetFat(), e);
            delete dir;
            dir = subDir;
        }

        string name = path->GetItem(i);
        e = dir->FindEntry(name.c_str());
        if (e == nullptr) {
            std::ostringstream msgStream;
            msgStream << "Couldn't find '" << path->ToString(false) << "': No such file or directory.";
            throw std::runtime_error(msgStream.str());
        }

        this->parentCchDir = dir;
        this->entry = e;
    }
}

Directory* Directory::GetRoot(FileSystem *fs)
{
    return new Directory(fs, new Path());
}

ClusterChainDirectory* Directory::GetCchDirectory() const
{
    if (path->IsRoot()) {
        return this->fs->GetRootDirectory();
    } else {
        return ClusterChainDirectory::GetDirectory(this->fs->GetDevice(), this->fs->GetFat(), this->entry);
    }
}

Directory::~Directory()
{
    if (this->parentCchDir != nullptr) {
        delete this->parentCchDir; // delete this->entry;  will be invoked automatically;
    }

    if (this->path->IsRoot()) {
        // Deallocate memory occupied by the fake root entry;
        delete this->entry;
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
    auto cchDir = this->GetCchDirectory();
    std::vector<DirectoryEntry *> *entries = cchDir->GetEntries();
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

    delete cchDir;
}

void Directory::GetFiles(std::vector<File*>& container) const
{
    auto cchDir = this->GetCchDirectory();
    std::vector<DirectoryEntry *> *entries = cchDir->GetEntries();
    for (size_t i = 0; i < entries->size(); i++) {
        DirectoryEntry *e = entries->at(i);
        if (e->IsFile()) {
            char nameBuf[256];
            e->GetName(nameBuf);
            Path *filePath = this->path->Clone();
            filePath->Combine(nameBuf);
            File *file = new File(this->fs, filePath);
            container.push_back(file);
        }
    }

    delete cchDir;
}

string Directory::GetName() const
{
    char nameBuf[256];
    if (this->path->IsRoot()) {
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
    //const char *cname = name.c_str();
    auto cchDir = this->GetCchDirectory();
    DirectoryEntry *subEntry = cchDir->AddDirectory(name.c_str(), this->fs->GetDevice());

    // Update Created/Modified time for the '..' directory
    ClusterChainDirectory *subDir = ClusterChainDirectory::GetDirectory(this->fs->GetDevice(), this->fs->GetFat(), subEntry);
    DirectoryEntry *parentEntry = subDir->FindEntry("..");

    time_t createdTime = this->entry->GetCreatedTime();
    time_t modifiedTime = this->entry->GetLastModifiedTime();
    parentEntry->SetCreatedTime(createdTime);
    parentEntry->SetLastModifiedTime(modifiedTime);

    subDir->Write(this->fs->GetDevice());

    delete subDir;
    delete cchDir;
}

void Directory::CreateFile(std::string name) const
{
    //const char *cname = name.c_str();
    auto cchDir = this->GetCchDirectory();
    cchDir->AddFile(name.c_str(), this->fs->GetDevice());
    delete cchDir;
}

File* Directory::GetFile(string path) const
{
    Path *filePath = this->path->Clone();
    filePath->Combine(path);
    return new File(this->fs, filePath);
}

void Directory::DeleteDirectory(string name) const
{
    auto cchDir = this->GetCchDirectory();
    cchDir->RemoveDirectory(name.c_str(), this->fs->GetDevice());
    delete cchDir;
}

void Directory::DeleteFile(string name) const
{
    auto cchDir = this->GetCchDirectory();
    cchDir->RemoveFile(name.c_str(), this->fs->GetDevice());
    delete cchDir;
}

void Directory::Write() const
{
    throw std::runtime_error("The operation is not supported.");
}

void Directory::Move(string srcPath, string destPath)
{
    FileDisk *dev = this->fs->GetDevice();
    Fat *fat = this->fs->GetFat();

    Path *srcPathObj = this->path->Clone();
    srcPathObj->Combine(srcPath);

    ClusterChainDirectory *srcDir = this->fs->GetRootDirectory();
    DirectoryEntry *srcEntry = nullptr;
    size_t i = 0;
    for (; i < srcPathObj->GetItemCount() - 1; i++) {
        string name = srcPathObj->GetItem(i);
        srcEntry = srcDir->FindEntry(name.c_str());
        if (srcEntry == nullptr) {
            std::ostringstream msgStream;
            msgStream << "Couldn't find '" << srcPathObj->ToString() << "': No such file or directory.";
            throw std::runtime_error(msgStream.str());
        }

        ClusterChainDirectory *subDir = ClusterChainDirectory::GetDirectory(dev, fat, srcEntry);
        delete srcDir;

        srcDir = subDir;
    }

    string srcName = srcPathObj->GetItem(i);
    srcEntry = srcDir->FindEntry(srcName.c_str());
    if (srcEntry == nullptr) {
        std::ostringstream msgStream;
        msgStream << "Couldn't find '" << srcPathObj->ToString() << "': No such file or directory.";
        throw std::runtime_error(msgStream.str());
    }

    Path *destPathObj = this->path->Clone();
    destPathObj->Combine(destPath);

    ClusterChainDirectory *destDir = this->fs->GetRootDirectory();
    DirectoryEntry *destEntry = nullptr;
    i = 0;
    for (; i < destPathObj->GetItemCount() - 1; i++) {
        string name = destPathObj->GetItem(i);
        DirectoryEntry *destEntry = destDir->FindEntry(name.c_str());
        if (destEntry == nullptr) {
            std::ostringstream msgStream;
            msgStream << "Couldn't find '" << destPathObj->ToString() << "': No such file or directory.";
            throw std::runtime_error(msgStream.str());
        }

        ClusterChainDirectory *subDir = ClusterChainDirectory::GetDirectory(dev, fat, destEntry);
        delete destDir;
        destDir = subDir;
    }    

    string destName = destPathObj->GetItem(i);
    destEntry = destDir->FindEntry(destName.c_str());

    if (srcEntry->IsFile()) {
        if (destEntry == nullptr) {
            // Move file with the new name;
            this->Move(srcDir, srcEntry, destDir, destName);
        } else if (destEntry->IsFile()) {
            destDir->RemoveFile(destName.c_str(), this->fs->GetDevice());
            this->Move(srcDir, srcEntry, destDir, destName);
            cout << "File '" << destPathObj->ToString() << "' has been replaced." << endl;
        } else {
            // Jump to the sub-directory;
            ClusterChainDirectory *subDir = ClusterChainDirectory::GetDirectory(dev, fat, destEntry);
            delete destDir;
            destDir = subDir;

            // Move the source file with the original name;
            this->Move(srcDir, srcEntry, destDir, srcName);
        }
    } else {
        if (destEntry == nullptr) {
            // Move dir with the new name;
            this->Move(srcDir, srcEntry, destDir, destName);
        } else {
            if (!destEntry->IsDir()) {
                std::ostringstream msgStream;
                msgStream << "Couldn't find '" << destPathObj->ToString() << "': No such directory.";
                throw std::runtime_error(msgStream.str());
            }

            // Jump to the sub-directory;
            ClusterChainDirectory *subDir = ClusterChainDirectory::GetDirectory(dev, fat, destEntry);
            delete destDir;
            destDir = subDir;

            // Get the source directory name;
            Path *srcNormalizedPathObj = this->path->Clone();
            srcNormalizedPathObj->Combine(srcPath, true);
            string srcDirName = srcNormalizedPathObj->GetItem(srcNormalizedPathObj->GetItemCount() - 1);

            // Move the source directory to the destination directory;
            this->Move(srcDir, srcEntry, destDir, srcDirName);

            delete srcNormalizedPathObj;
        }
    }

    delete srcPathObj;
    delete destPathObj;

    delete srcDir;
    delete destDir;
}

void Directory::Move(ClusterChainDirectory *srcDir, DirectoryEntry *srcEntry, ClusterChainDirectory *destDir, string destName)
{
    FileDisk *dev = this->fs->GetDevice();
    const char *newName = destName.c_str();
    if (srcDir->GetStartCluster() == destDir->GetStartCluster()) {
        // Rename file or directory;
        srcDir->SetName(dev, srcEntry, newName);
    } else {
        srcDir->Move(dev, srcEntry, destDir, newName);
    }
}

tm* Directory::GetCreatedTime() const
{
    time_t time = this->entry->GetCreatedTime();
    return localtime(&time);
}

tm* Directory::GetLastModifiedTime() const
{
    time_t time = this->entry->GetLastModifiedTime();
    return localtime(&time);
}

void Directory::ImportFile(string path)
{
    FILE *fp = fopen(path.c_str(), "r+b");
    if (fp == nullptr) {
        std::ostringstream msgStream;
        msgStream << "Couldn't open the file '" << path << "'";
        std::error_code errorCode(errno, std::generic_category());
        throw std::ios_base::failure(msgStream.str(), errorCode);
    }

    Path pathObj;
    pathObj.Combine(path);
    string fileName = pathObj.GetItem(pathObj.GetItemCount() - 1);

    this->CreateFile(fileName);
    File *file = this->GetFile(fileName);

    const size_t BUFFER_SIZE = 4096;
    uint8_t buf[BUFFER_SIZE];

    uint32_t offset = 0;
    fseek(fp, 0, SEEK_SET);
    size_t nread = fread(buf, sizeof(char), BUFFER_SIZE, fp);
    while (nread > 0) {
        file->Write(offset, nread, buf);
        offset += nread;
        nread = fread(buf, sizeof(char), BUFFER_SIZE, fp);
    }

    fclose(fp);
    delete file;
}

void Directory::ImportDirectory(string path)
{
    DIR *dir = opendir(path.c_str());
    if (dir == NULL) {
        std::ostringstream msgStream;
        msgStream << "Couldn't open the directory '" << path << "'.";
        throw std::ios_base::failure(msgStream.str());
    }

    struct dirent *ent;
    while ((ent = readdir (dir)) != NULL) {
        try {
            if (ent->d_type == DT_REG) {
                // Import a regular file;
                string filePath = path + "/" + ent->d_name;
                this->ImportFile(filePath);
            } else if (ent->d_type == DT_DIR) {
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                    // Import a sub-directory;
                    this->CreateDirectory(ent->d_name);
                    Directory *subDir = this->GetDirectory(ent->d_name);
                    string subDirPath = path + "/" + ent->d_name;
                    subDir->ImportDirectory(subDirPath);
                    delete subDir;
                }
            }
        } catch (const std::ios_base::failure& error) {
            cout << error.what() << endl;
        }
    }

    closedir(dir);
}

void Directory::Import(string path)
{
    struct stat info;
    stat(path.c_str(), &info);
    if (S_ISREG(info.st_mode)) {
        this->ImportFile(path);
    } else if (S_ISDIR(info.st_mode)) {
        this->ImportDirectory(path);
    } else {
        std::ostringstream msgStream;
        msgStream << "Couldn't open '" << path << "'. No such file or directory.";
        throw std::ios_base::failure(msgStream.str());
    }
}
