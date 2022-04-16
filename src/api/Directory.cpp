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

Directory::Directory()
    : fs(nullptr)
{ }

Directory::Directory(FileSystem *fs, Path&& path)
    : fs(fs), path(std::move(path))
{
    Init();
}

Directory::Directory(FileSystem *fs, Path& path)
    : fs(fs), path(path)
{
    Init();
}

void Directory::Init() 
{
   if (path.IsRoot()) {
        time_t now = time(0);

        // Fake root entry;
        this->entry.SetStartCluster(fs->GetBootSector()->GetRootDirFirstCluster());
        this->entry.SetCreatedTime(now);
        this->entry.SetLastModifiedTime(now);
        this->entry.SetIsDir(true);
    } else {
        ClusterChainDirectory dir = fs->GetRootDirectory();
        DirectoryEntry e;
        size_t i = 0;
        for (; i < path.GetItemCount() - 1; ++i) {
            std::string name = path.GetItem(i);
            auto eIdx = dir.FindEntryIndex(name.c_str());
            if (eIdx == -1) {
                std::ostringstream msgStream;
                msgStream << "Couldn't find '" << path.ToString(false) << "': No such file or directory";
                throw std::ios_base::failure(msgStream.str());                
            }

            e = dir.FindEntry(name.c_str());
            ClusterChainDirectory subDir = ClusterChainDirectory::GetDirectory(fs->GetDevice(), fs->GetFat(), e);
            dir = std::move(subDir);
        }

        std::string name = path.GetItem(i);
        auto eIdx = dir.FindEntryIndex(name.c_str());
        if (eIdx == -1) {
            std::ostringstream msgStream;
            msgStream << "Couldn't find '" << path.ToString(false) << "': No such file or directory";
            throw std::ios_base::failure(msgStream.str());
        }

        e = dir.FindEntry(name.c_str());

        this->parentCchDir = std::move(dir);
        this->entry = std::move(e);
    }
}

Directory::Directory(const Directory& other) :
    fs(other.fs),
    parentCchDir(other.parentCchDir),
    entry(other.entry),
    path(other.path)
{
}

Directory::Directory(Directory&& other) :
    fs(std::exchange(other.fs, nullptr)),
    parentCchDir(std::move(other.parentCchDir)),
    entry(std::move(other.entry)),
    path(std::move(other.path))
{
}

Directory& Directory::operator=(const Directory& other)
{
    if (this != &other) {
        Cleanup();
        
        fs = other.fs;
        parentCchDir = other.parentCchDir;
        entry = other.entry;
        path = other.path;
    }
    return *this;
}

Directory& Directory::operator=(Directory&& other)
{
    if (this != &other) {
        Cleanup();

        fs = std::exchange(other.fs, nullptr);
        parentCchDir = std::move(other.parentCchDir);
        entry = std::move(other.entry);
        path = std::move(other.path);
    }
    return *this;
}

Directory Directory::GetRoot(FileSystem *fs)
{
    return Directory(fs, std::move(Path()));
}

ClusterChainDirectory Directory::GetCchDirectory() const
{
    if (path.IsRoot()) {
        return this->fs->GetRootDirectory();
    } else {
        return ClusterChainDirectory::GetDirectory(this->fs->GetDevice(), this->fs->GetFat(), this->entry);
    }
}

void Directory::Cleanup()
{
}

Directory::~Directory()
{
    Cleanup();
}

bool Directory::IsRoot() const
{
    return this->entry.GetStartCluster() == this->fs->GetBootSector()->GetRootDirFirstCluster();
}

std::vector<Directory> Directory::GetDirectories() const
{
    auto cchDir = GetCchDirectory();
    std::vector<DirectoryEntry> entries = cchDir.GetEntries();
    size_t dirCount = 0;
    for (size_t i = 0; i < entries.size(); ++i) {
        DirectoryEntry& e = entries[i];
        if (e.IsDir()) {
            ++dirCount;
        }
    }
    
    std::vector<Directory> result(dirCount);
    
    size_t dirIdx = 0;
    for (size_t i = 0; i < entries.size(); ++i) {
        DirectoryEntry& e = entries[i];
        if (e.IsDir()) {
            char nameBuf[256];
            e.GetName(nameBuf);
            Path dirPath(this->path);
            dirPath.Combine(nameBuf);
            Directory dir(this->fs, std::move(dirPath));
            result[dirIdx++] = std::move(dir);
        }
    }

    return result;
}

std::vector<File> Directory::GetFiles() const
{
    auto cchDir = this->GetCchDirectory();
    std::vector<DirectoryEntry> entries = cchDir.GetEntries();
    size_t fileCount = 0;
    for (size_t i = 0; i < entries.size(); ++i) {
        DirectoryEntry& e = entries[i];
        if (e.IsFile()) {
            ++fileCount;
        }
    }

    std::vector<File> result(fileCount);
    size_t fileIdx = 0;
    for (size_t i = 0; i < entries.size(); ++i) {
        DirectoryEntry& e = entries[i];
        if (e.IsFile()) {
            char nameBuf[256];
            e.GetName(nameBuf);
            Path filePath(this->path);
            filePath.Combine(nameBuf);
            File file(this->fs, std::move(filePath));
            result[fileIdx++] = std::move(file);
        }
    }
    
    return result;
}

std::string Directory::GetName() const
{
    if (this->path.IsRoot()) {
        return "/";
    }

    char nameBuf[256];
    this->entry.GetName(nameBuf);
    std::string s(nameBuf);
    return s;
}

Directory Directory::GetDirectory(const std::string& path) const
{
    Path dirPath(this->path);
    dirPath.Combine(path);
    return Directory(this->fs, std::move(dirPath));
}

void Directory::CreateDirectory(const std::string& name) const
{
    auto cchDir = this->GetCchDirectory();
    DirectoryEntry& subEntry = cchDir.AddDirectory(name.c_str(), this->fs->GetDevice());

    // Update Created/Modified time for the '..' directory
    ClusterChainDirectory subDir = ClusterChainDirectory::GetDirectory(this->fs->GetDevice(), this->fs->GetFat(), subEntry);
    DirectoryEntry& parentEntry = subDir.FindEntry("..");

    time_t createdTime = this->entry.GetCreatedTime();
    time_t modifiedTime = this->entry.GetLastModifiedTime();
    
    parentEntry.SetCreatedTime(createdTime);
    parentEntry.SetLastModifiedTime(modifiedTime);

    subDir.Write(this->fs->GetDevice());
}

void Directory::CreateFile(const std::string& name) const
{
    auto cchDir = this->GetCchDirectory();
    cchDir.AddFile(name.c_str(), this->fs->GetDevice());
}

File Directory::GetFile(const std::string& path) const
{
    Path filePath(this->path);
    filePath.Combine(path);
    return File(this->fs, std::move(filePath));
}

void Directory::DeleteDirectory(const std::string& path) const
{
    Path pathObj(this->path);
    pathObj.Combine(path);
    std::string dirName = pathObj.GetItem(pathObj.GetItemCount() - 1);
    Directory parentDir(this->fs, std::move(pathObj.GetParent()));

    // Remove a sub directory;
    auto parentCchDir = parentDir.GetCchDirectory();
    parentCchDir.RemoveDirectory(dirName.c_str(), this->fs->GetDevice());
}

void Directory::DeleteFile(const std::string& path) const
{
    Path pathObj(this->path);
    pathObj.Combine(path);
    std::string fileName = pathObj.GetItem(pathObj.GetItemCount() - 1);
    Directory parentDir(this->fs, std::move(pathObj.GetParent()));

    // Remove;
    auto parentCchDir = parentDir.GetCchDirectory();
    parentCchDir.RemoveFile(fileName.c_str(), this->fs->GetDevice());
}

void Directory::Write() const
{
    throw std::runtime_error("The operation is not supported.");
}

void Directory::Move(const std::string& srcPath, const std::string& destPath)
{
    Device& dev = this->fs->GetDevice();
    Fat *fat = this->fs->GetFat();

    Path srcPathObj(this->path);
    srcPathObj.Combine(srcPath);

    ClusterChainDirectory srcDir = this->fs->GetRootDirectory();
    DirectoryEntry srcEntry;
    size_t i = 0;
    for (; i < srcPathObj.GetItemCount() - 1; ++i) {
        std::string name = srcPathObj.GetItem(i);
        auto srcEntryIdx = srcDir.FindEntryIndex(name.c_str());
        if (srcEntryIdx == -1) {
            std::ostringstream msgStream;
            msgStream << "Couldn't find '" << srcPathObj.ToString() << "': No such file or directory.";
            throw std::runtime_error(msgStream.str());
        }

        srcEntry = srcDir.FindEntry(name.c_str());
        ClusterChainDirectory subDir = ClusterChainDirectory::GetDirectory(dev, fat, srcEntry);
        srcDir = std::move(subDir);
    }

    std::string srcName = srcPathObj.GetItem(i);
    auto srcEntryIdx = srcDir.FindEntryIndex(srcName.c_str());
    if (srcEntryIdx == -1) {
        std::ostringstream msgStream;
        msgStream << "Couldn't find '" << srcPathObj.ToString() << "': No such file or directory.";
        throw std::runtime_error(msgStream.str());    
    }

    srcEntry = srcDir.FindEntry(srcName.c_str());
    
    Path destPathObj(this->path);
    destPathObj.Combine(destPath);

    ClusterChainDirectory destDir = this->fs->GetRootDirectory();
    DirectoryEntry destEntry;
    i = 0;
    for (; i < destPathObj.GetItemCount() - 1; ++i) {
        std::string name = destPathObj.GetItem(i);
        auto destEntryIdx = destDir.FindEntryIndex(name.c_str());
        if (destEntryIdx == -1) {
            std::ostringstream msgStream;
            msgStream << "Couldn't find '" << destPathObj.ToString() << "': No such file or directory.";
            throw std::runtime_error(msgStream.str());
        }

        destEntry = destDir.FindEntry(name.c_str());
        ClusterChainDirectory subDir = ClusterChainDirectory::GetDirectory(dev, fat, destEntry);
        destDir = std::move(subDir);
    }    

    std::string destName = destPathObj.GetItem(i);
    auto destEntryIdx = destDir.FindEntryIndex(destName.c_str());

    if (srcEntry.IsFile()) {
        if (destEntryIdx == -1) {
            // Move the source file with the new name;
            this->Move(srcDir, srcEntry, destDir, destName);
        } else {
            destEntry = destDir.FindEntry(destName.c_str());
            if (destEntry.IsFile()) {
                destDir.RemoveFile(destName.c_str(), dev);
                this->Move(srcDir, srcEntry, destDir, destName);
                cout << "File '" << destPathObj.ToString() << "' has been replaced." << endl;
            } else {
                // Jump to the sub-directory;
                ClusterChainDirectory subDir = ClusterChainDirectory::GetDirectory(dev, fat, destEntry);
                destDir = std::move(subDir);

                // Move the source file with the original name;
                this->Move(srcDir, srcEntry, destDir, srcName);
            }
        }
    } else {
        if (destEntryIdx == -1) {
            std::ostringstream msgStream;
            msgStream << "Couldn't find '" << destPathObj.ToString() << "': No such directory.";
            throw std::runtime_error(msgStream.str());
        }
        
        destEntry = destDir.FindEntry(destName.c_str());
        if (!destEntry.IsDir()) {
            std::ostringstream msgStream;
            msgStream << "'" << destPathObj.ToString() << "': Not a directory.";
            throw std::runtime_error(msgStream.str());
        }

        // Jump to the sub-directory;
        ClusterChainDirectory subDir = ClusterChainDirectory::GetDirectory(dev, fat, destEntry);
        destDir = std::move(subDir);

        // Get the source directory name;
        Path srcNormalizedPathObj(this->path);
        srcNormalizedPathObj.Combine(srcPath, true);
        std::string srcDirName = srcNormalizedPathObj.GetItem(srcNormalizedPathObj.GetItemCount() - 1);

        // Move the source directory to the destination directory;
        this->Move(srcDir, srcEntry, destDir, srcDirName);
    }
}

void Directory::Move(ClusterChainDirectory& srcDir, DirectoryEntry& srcEntry, ClusterChainDirectory& destDir, const std::string& destName)
{
    Device& dev = this->fs->GetDevice();
    const char *newName = destName.c_str();
    if (srcDir.GetStartCluster() == destDir.GetStartCluster()) {
        // Rename file or directory;
        srcDir.SetName(dev, srcEntry, newName);
    } else {
        srcDir.Move(dev, srcEntry, destDir, newName);
    }
}

tm* Directory::GetCreatedTime() const
{
    time_t time = this->entry.GetCreatedTime();
    return localtime(&time);
}

tm* Directory::GetModifiedTime() const
{
    time_t time = this->entry.GetLastModifiedTime();
    return localtime(&time);
}

void Directory::ImportFile(const std::string& path)
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
    std::string fileName = pathObj.GetItem(pathObj.GetItemCount() - 1);

    CreateFile(fileName);
    File file = GetFile(fileName);

    const size_t BUFFER_SIZE = 4096;
    uint8_t buf[BUFFER_SIZE];

    uint32_t offset = 0;
    fseek(fp, 0, SEEK_SET);
    size_t nread = fread(buf, sizeof(char), BUFFER_SIZE, fp);
    while (nread > 0) {
        file.Write(offset, nread, buf);
        offset += nread;
        nread = fread(buf, sizeof(char), BUFFER_SIZE, fp);
    }

    fclose(fp);
}

void Directory::ImportDirectory(const std::string& path)
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
                std::string filePath = path + "/" + ent->d_name;
                this->ImportFile(filePath);
            } else if (ent->d_type == DT_DIR) {
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                    // Import a sub-directory;
                    this->CreateDirectory(ent->d_name);
                    Directory subDir = this->GetDirectory(ent->d_name);
                    std::string subDirPath = path + "/" + ent->d_name;
                    subDir.ImportDirectory(subDirPath);
                }
            }
        } catch (const std::ios_base::failure& error) {
            cout << error.what() << endl;
        }
    }

    closedir(dir);
}

void Directory::Import(const std::string& path)
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

void Directory::Copy(const std::string& srcPath, const std::string& destPath)
{
    Device& dev = this->fs->GetDevice();
    Fat *fat = this->fs->GetFat();

    Path srcPathObj(this->path);
    srcPathObj.Combine(srcPath);

    ClusterChainDirectory srcDir = this->fs->GetRootDirectory();
    for (size_t i = 0; i < srcPathObj.GetItemCount() - 1; ++i) {
        std::string name = srcPathObj.GetItem(i);
        auto srcEntryIdx = srcDir.FindEntryIndex(name.c_str());
        if (srcEntryIdx == -1) {
            std::ostringstream msgStream;
            msgStream << "Couldn't find '" << srcPathObj.ToString() << "': No such file or directory.";
            throw std::runtime_error(msgStream.str());
        }

        DirectoryEntry& srcEntry = srcDir.FindEntry(name.c_str());
        ClusterChainDirectory subDir = ClusterChainDirectory::GetDirectory(dev, fat, srcEntry);
        srcDir = std::move(subDir);
    }

    std::string srcName = srcPathObj.GetLastItem();
    auto srcEntryIdx = srcDir.FindEntryIndex(srcName.c_str());
    if (srcEntryIdx == -1) {
        std::ostringstream msgStream;
        msgStream << "Couldn't find '" << srcPathObj.ToString() << "': No such file or directory.";
        throw std::runtime_error(msgStream.str());
    }
    
    DirectoryEntry& srcEntry = srcDir.FindEntry(srcName.c_str());

    Path destPathObj(this->path);
    destPathObj.Combine(destPath);

    ClusterChainDirectory destDir = this->fs->GetRootDirectory();   
    for (size_t i = 0; i < destPathObj.GetItemCount() - 1; ++i) {
        std::string name = destPathObj.GetItem(i);
        auto destEntryIdx = destDir.FindEntryIndex(name.c_str());
        if (destEntryIdx == -1) {
            std::ostringstream msgStream;
            msgStream << "Couldn't find '" << destPathObj.ToString() << "': No such file or directory.";
            throw std::runtime_error(msgStream.str());
        }

        DirectoryEntry& destEntry = destDir.FindEntry(name.c_str());
        ClusterChainDirectory subDir = ClusterChainDirectory::GetDirectory(dev, fat, destEntry);
        destDir = std::move(subDir);
    }

    std::string destName = destPathObj.GetLastItem();
    auto destEntryIdx = destDir.FindEntryIndex(destName.c_str());

    if (srcEntry.IsFile()) {
        if (destEntryIdx == -1) {
            // Copy the source file with the new name;
            this->CopyFile(srcDir, srcEntry, destDir, destName);
        } else {
            DirectoryEntry& destEntry = destDir.FindEntry(destName.c_str());
            if (destEntry.IsFile()) {
                destDir.RemoveFile(destName.c_str(), dev);
                this->CopyFile(srcDir, srcEntry, destDir, destName);
                cout << "File '" << destPathObj.ToString() << "' has been replaced." << endl;
            } else {
                // Jump to the sub-directory;
                ClusterChainDirectory subDir = ClusterChainDirectory::GetDirectory(dev, fat, destEntry);
                destDir = std::move(subDir);

                // Copy the source file with its original name;
                this->CopyFile(srcDir, srcEntry, destDir, srcName);
            }
        }
    } else {
        if (destEntryIdx == -1) {
            std::ostringstream msgStream;
            msgStream << "Couldn't find '" << destPathObj.ToString() << "': No such directory.";
            throw std::runtime_error(msgStream.str());
        }
        
        DirectoryEntry& destEntry = destDir.FindEntry(destName.c_str());
        if (!destEntry.IsDir()) {
            std::ostringstream msgStream;
            msgStream << "'" << destPathObj.ToString() << "': Not a directory.";
            throw std::runtime_error(msgStream.str());        
        }

        // Jump to the sub-directory;
        ClusterChainDirectory subDir = ClusterChainDirectory::GetDirectory(dev, fat, destEntry);
        destDir = std::move(subDir);

        // Get the source directory name;
        Path srcNormalizedPathObj(this->path);
        srcNormalizedPathObj.Combine(srcPath, true);
        std::string srcDirName = srcNormalizedPathObj.GetItem(srcNormalizedPathObj.GetItemCount() - 1);

        // Copy the source directory to the destination directory;
        this->CopyDirectory(srcDir, srcEntry, destDir, srcDirName);
    }
}

void Directory::CopyFile(ClusterChainDirectory& srcDir, DirectoryEntry& srcEntry, ClusterChainDirectory& destDir, const std::string& destName)
{
    Device& dev = this->fs->GetDevice();
    const char *newName = destName.c_str();
    if (srcDir.GetStartCluster() == destDir.GetStartCluster()) {
        // Copy to the same directory;
        destDir = srcDir;
    }

    srcDir.CopyFile(dev, srcEntry, destDir, newName);
    destDir.Write(this->fs->GetDevice());
}

void Directory::CopyDirectory(ClusterChainDirectory& srcDir, DirectoryEntry& srcEntry, ClusterChainDirectory& destDir, const std::string& destName)
{
    Device& dev = this->fs->GetDevice();
    const char *newName = destName.c_str();
    if (srcDir.GetStartCluster() == destDir.GetStartCluster()) {
        // Copy to the same directory;
        destDir = srcDir;
    }

    srcDir.CopyDirectory(dev, srcEntry, destDir, newName);
}
