#include "../../include/Common.h"
#include "../../include/api/File.h"
#include "../../include/api/FileSystem.h"

using namespace org::vfat;
using namespace org::vfat::api;

File::File(FileSystem *fs, ClusterChainDirectory *parentDir, DirectoryEntry *entry, Path *path)
{
    this->fs = fs;
    this->path = path;
    this->parentCchDir = parentDir;
    this->entry = entry;
}

File::~File()
{
    delete this->path;
}

uint32_t File::GetSize() const
{
    return this->entry->GetDataLength();
}

string File::GetName() const
{
    char nameBuf[256];
    this->entry->GetName(nameBuf);
    string s(nameBuf);
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
    delete cchFile;
}

string File::ReadText(uint32_t offset, uint32_t nchars) const
{
    // Read raw data;
    uint8_t *buf = new uint8_t[nchars];
    this->Read(offset, nchars, buf);

    // Create C string;
    char *cstr = new char[nchars + 1];
    memcpy(cstr, buf, nchars);
    cstr[nchars] = '\0';

    // Create C++ string;
    string str(cstr);

    // Deallocate memory allocated for buffers because they are not used any longer;
    delete[] buf;
    delete[] cstr;

    return str;
}

void File::WriteText(string s, uint32_t offset) const
{
    const char *cstr = s.c_str();
    uint8_t *buf = new uint8_t[s.size()];
    memcpy(buf, cstr, s.size());
    this->Write(offset, s.size(), buf);
}
