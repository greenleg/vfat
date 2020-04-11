#include "../../include/api/File.h"
#include "../../include/api/FileSystem.h"

using namespace org::vfat;
using namespace org::vfat::api;

File::File(FileSystem *fs, DirectoryEntry *e)
{
    this->fs = fs;
    this->entry = e;
}

File::~File()
{
    delete this->entry;
}

string File::GetName() const
{
    char nameBuf[256];
    this->entry->GetName(nameBuf);
    string s(nameBuf);
    return s; // return a copy of the local variable s;
}

uint32_t File::Read(uint32_t offset, uint32_t nbytes, uint8_t *buffer)
{
    ClusterChainFile *cchFile = ClusterChainDirectory::GetFile(this->fs->GetFat(), this->entry);
    uint32_t nread = cchFile->Read(this->fs->GetDevice(), offset, nbytes, buffer);
    delete cchFile;

    return nread;
}

void File::Write(uint32_t offset, uint32_t nbytes, uint8_t *buffer)
{
    ClusterChainFile *cchFile = ClusterChainDirectory::GetFile(this->fs->GetFat(), this->entry);
    cchFile->Write(this->fs->GetDevice(), offset, nbytes, buffer);
    delete cchFile;
}
