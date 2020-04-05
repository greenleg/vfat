#include "../../include/api/Directory.h"

using namespace org::vfat;
using namespace org::vfat::api;

void Directory::GetDirectories(std::vector<Directory*>& container) const
{
    std::vector<DirectoryEntry *> *entries = this->cchDir->GetEntries();
    for (int i = 0; i < entries->size(); i++) {
        DirectoryEntry *e = entries->at(i);

    }
}
