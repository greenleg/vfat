#include <iostream>
#include <queue>
#include "../../include/Common.h"
#include "../../include/api/File.h"
#include "../../include/api/FileSystem.h"

using namespace org::vfat;
using namespace org::vfat::api;

File::File()
    : fs(nullptr), entry(nullptr)
{ }

File::File(FileSystem *fs, Path& path)
    : fs(fs), path(path) 
{
    Init();
}

File::File(FileSystem *fs, Path&& path)
    : fs(fs), path(std::move(path)) 
{
    Init();
}

void File::Init()
{
    std::queue<ClusterChainDirectory> subDirectories;
    ClusterChainDirectory dir = fs->GetRootDirectory();
    DirectoryEntry *e;
    size_t i = 0;
    for (; i < path.GetItemCount() - 1; ++i) {
        std::string name = this->path.GetItem(i);
        e = dir.FindEntry(name.c_str());
        if (e == nullptr) {
            std::ostringstream msgStream;
            msgStream << "Couldn't find '" << path.ToString() << "': No such file or directory";
            throw std::ios_base::failure(msgStream.str());
        }

        ClusterChainDirectory subDir = ClusterChainDirectory::GetDirectory(fs->GetDevice(), fs->GetFat(), e);
        dir = std::move(subDir);
    }

    std::string name = path.GetItem(i);
    e = dir.FindEntry(name.c_str());
    if (e == nullptr) {
        std::ostringstream msgStream;
        msgStream << "Couldn't find '" << path.ToString() << "': No such file or directory";
        throw std::ios::failure(msgStream.str());
    }

    this->parentCchDir = std::move(dir);
    this->entry = e;
}

File::File(const File& other) :
    fs(other.fs),
    parentCchDir(other.parentCchDir),
    entry(other.entry),
    path(other.path)
{ }

File::File(File&& other) :
    fs(std::exchange(other.fs, nullptr)),
    parentCchDir(std::move(other.parentCchDir)),
    entry(std::exchange(other.entry, nullptr)),
    path(std::move(other.path))
{ }

File& File::operator=(const File& other)
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

File& File::operator=(File&& other)
{
    if (this != &other) {
        Cleanup();

        fs = std::exchange(other.fs, nullptr);
        parentCchDir = std::move(other.parentCchDir);
        entry = std::exchange(other.entry, nullptr);
        path = std::move(other.path);
    }
    return *this;
}

void File::Cleanup()
{
//    if (parentCchDir != nullptr) {
//        delete parentCchDir;
//    }
}

File::~File()
{
    Cleanup();
}

uint32_t File::GetSize() const
{
    return this->entry->GetDataLength();
}

std::string File::GetName() const
{
    char nameBuf[256];
    this->entry->GetName(nameBuf);
    std::string s(nameBuf);
    return s; // return a copy of the local variable s;
}

uint32_t File::Read(uint32_t offset, uint32_t nbytes, uint8_t *buffer) const
{
    ClusterChainFile *cchFile = ClusterChainDirectory::GetFile(this->fs->GetFat(), this->entry);
    uint32_t nread = cchFile->Read(this->fs->GetDevice(), offset, nbytes, buffer);
    delete cchFile;

    return nread;
}

void File::Write(uint32_t offset, uint32_t nbytes, uint8_t *buffer) const
{
    ClusterChainFile *cchFile = ClusterChainDirectory::GetFile(this->fs->GetFat(), this->entry);
    cchFile->Write(this->fs->GetDevice(), offset, nbytes, buffer);

    // The parent directory contains information about a file including name, size, creation time etc.
    // This updated information should be stored to a device as well.
    this->parentCchDir.Write(this->fs->GetDevice());

    delete cchFile;
}

std::string File::ReadText(uint32_t offset, uint32_t nchars) const
{
    // Read raw data;
    uint8_t *buf = new uint8_t[nchars];
    this->Read(offset, nchars, buf);

    // Create C string;
    char *cstr = new char[nchars + 1];
    memcpy(cstr, buf, nchars);
    cstr[nchars] = '\0';

    // Create C++ string;
    std::string str(cstr);

    // Deallocate memory allocated for buffers because they are not used any longer;
    delete[] buf;
    delete[] cstr;

    return str;
}

void File::WriteText(const std::string& s, uint32_t offset) const
{
    const char *cstr = s.c_str();
    uint8_t *buf = new uint8_t[s.size()];
    memcpy(buf, cstr, s.size());
    this->Write(offset, s.size(), buf);
}

tm* File::GetCreatedTime() const
{
    time_t time = this->entry->GetCreatedTime();
    return localtime(&time);
}

tm* File::GetModifiedTime() const
{
    time_t time = this->entry->GetLastModifiedTime();
    return localtime(&time);
}
