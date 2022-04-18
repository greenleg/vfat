#include <iostream>
#include <queue>
#include "../../include/Common.h"
#include "../../include/api/File.h"
#include "../../include/api/FileSystem.h"

using namespace org::vfat;
using namespace org::vfat::api;

File::File()
    : fs(nullptr)
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
    ClusterChainDirectory dir = fs->GetRootDirectory();
    for (size_t i = 0; i < path.GetItemCount() - 1; ++i) {
        const std::string& name = this->path.GetItem(i);
        auto eIdx = dir.FindEntryIndex(name.c_str());
        if (eIdx == -1) {
            std::ostringstream msgStream;
            msgStream << "Couldn't find '" << path.ToString() << "': No such file or directory";
            throw std::ios_base::failure(msgStream.str());
        }

        DirectoryEntry& e = dir.FindEntry(name.c_str());
        ClusterChainDirectory subDir = ClusterChainDirectory::GetDirectory(fs->GetDevice(), fs->GetFat(), e);
        dir = std::move(subDir);
    }

    const std::string& name = path.GetLastItem();
    auto eIdx = dir.FindEntryIndex(name.c_str());
    if (eIdx == -1) {
        std::ostringstream msgStream;
        msgStream << "Couldn't find '" << path.ToString() << "': No such file or directory";
        throw std::ios::failure(msgStream.str());
    }

    this->parentCchDir = std::move(dir);
}

File::File(const File& other) :
    fs(other.fs),
    parentCchDir(other.parentCchDir),
    path(other.path)
{ }

File::File(File&& other) :
    fs(std::exchange(other.fs, nullptr)),
    parentCchDir(std::move(other.parentCchDir)),
    path(std::move(other.path))
{ }

File& File::operator=(const File& other)
{
    if (this != &other) {
        Cleanup();
        
        fs = other.fs;
        parentCchDir = other.parentCchDir;
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
        path = std::move(other.path);
    }
    return *this;
}

void File::Cleanup()
{
}

File::~File()
{
    Cleanup();
}

uint32_t File::GetSize() const
{
    const std::string& fileName = this->path.GetLastItem();
    const DirectoryEntry& entry = parentCchDir.FindEntry(fileName.c_str());
    return entry.GetDataLength();
}

std::string File::GetName() const
{
    return this->path.GetLastItem();
}

uint32_t File::Read(uint32_t offset, uint32_t nbytes, uint8_t *buffer) const
{
    const std::string& fileName = this->path.GetLastItem();
    const DirectoryEntry& entry = this->parentCchDir.FindEntry(fileName.c_str());
    ClusterChainFile cchFile = ClusterChainDirectory::GetFile(entry);
    uint32_t nread = cchFile.Read(this->fs->GetDevice(), this->fs->GetFat(), offset, nbytes, buffer);

    return nread;
}

void File::Write(uint32_t offset, uint32_t nbytes, uint8_t *buffer)
{
    const std::string& fileName = this->path.GetLastItem();
    DirectoryEntry& entry = parentCchDir.FindEntry(fileName.c_str());
    ClusterChainFile cchFile = ClusterChainDirectory::GetFile(entry);
    cchFile.Write(this->fs->GetDevice(), this->fs->GetFat(), offset, nbytes, buffer);
    entry = cchFile.GetEntry();
    // The parent directory contains information about a file including name, size, creation time etc.
    // This updated information should be stored to a device as well.
    this->parentCchDir.Write(this->fs->GetDevice(), this->fs->GetFat());
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

void File::WriteText(const std::string& s, uint32_t offset)
{
    const char *cstr = s.c_str();
    uint8_t *buf = new uint8_t[s.size()];
    memcpy(buf, cstr, s.size());
    this->Write(offset, s.size(), buf);
}

tm* File::GetCreatedTime() const
{
    const std::string& fileName = this->path.GetLastItem();
    const DirectoryEntry& entry = this->parentCchDir.FindEntry(fileName.c_str());
    time_t time = entry.GetCreatedTime();
    return localtime(&time);
}

tm* File::GetModifiedTime() const
{
    const std::string& fileName = this->path.GetLastItem();
    const DirectoryEntry& entry = this->parentCchDir.FindEntry(fileName.c_str());
    time_t time = entry.GetLastModifiedTime();
    return localtime(&time);
}
