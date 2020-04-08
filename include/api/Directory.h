#ifndef VFAT_DIRECTORY_H
#define VFAT_DIRECTORY_H

#include <string>
#include "../ClusterChainDirectory.h"
#include "FileSystem.h"

using namespace std;
using namespace org::vfat;

namespace org::vfat::api
{
    class Directory
    {
    private:
        FileSystem *fs;
        DirectoryEntry *entry;        
        Directory(FileSystem *fs);

    public:        
        Directory(FileSystem *fs, DirectoryEntry *e);
        static Directory* GetRoot(FileSystem *fs);
        ~Directory();
        void GetDirectories(std::vector<Directory*>& container) const;
        string GetName() const;
    };
}

#endif // VFAT_DIRECTORY_H
