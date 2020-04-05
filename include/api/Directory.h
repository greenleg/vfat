#ifndef VFAT_DIRECTORY_H
#define VFAT_DIRECTORY_H

#include "../ClusterChainDirectory.h"

using namespace org::vfat;

namespace org::vfat::api
{
    class Directory
    {
    private:
        DirectoryEntry *entry;
        ClusterChainDirectory *cchDir;

    public:
        void GetDirectories(std::vector<Directory*>& container) const;
    };
}

#endif // VFAT_DIRECTORY_H
