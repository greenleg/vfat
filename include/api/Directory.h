#ifndef VFAT_DIRECTORY_H
#define VFAT_DIRECTORY_H

#include <string>
#include "../ClusterChainDirectory.h"
#include "FileSystem.h"
#include "File.h"

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
        bool IsRoot() const;

    public:        
        Directory(FileSystem *fs, DirectoryEntry *e);
        static Directory* GetRoot(FileSystem *fs);
        ~Directory();
        void GetDirectories(std::vector<Directory*>& container) const;
        void GetFiles(std::vector<File*>& container) const;
        void CreateFile(string name) const;
        void DeleteFile(string name) const;
        void CreateDirectory(string name) const;
        void DeleteDirectory(string name) const;
        File* GetFile(string name) const;
        Directory* GetDirectory(string name) const;
        Directory* ChangeDirectory(string path) const;
        string GetName() const;

        static void MoveFile(FileSystem *fs, File *file, Directory *dest);
        static void MoveDirectory(FileSystem *fs, Directory *dir, Directory *dest);
    };
}

#endif // VFAT_DIRECTORY_H
