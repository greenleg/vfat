#ifndef VFAT_DIRECTORY_H
#define VFAT_DIRECTORY_H

#include <string>
#include "../ClusterChainDirectory.h"
#include "FileSystem.h"
#include "File.h"
#include "Path.h"

using namespace std;
using namespace org::vfat;

namespace org::vfat::api
{
    class Directory
    {
    private:
        FileSystem *fs;
        ClusterChainDirectory *parentCchDir;
        DirectoryEntry *entry;
        //ClusterChainDirectory *cchDir;
        Path *path;

        bool IsRoot() const;
        void Move(ClusterChainDirectory *srcDir, DirectoryEntry *srcEntry, ClusterChainDirectory *destDir, string destName);
        void CopyFile(ClusterChainDirectory *srcDir, DirectoryEntry *srcEntry, ClusterChainDirectory *destDir, string destName);
        void CopyDirectory(ClusterChainDirectory *srcDir, DirectoryEntry *srcEntry, ClusterChainDirectory *destDir, string destName);
        void ImportFile(string path);
        void ImportDirectory(string path);
        ClusterChainDirectory* GetCchDirectory() const;

    public:
        Directory(FileSystem *fs, Path *path);
        static Directory* GetRoot(FileSystem *fs);
        ~Directory();
        void GetDirectories(std::vector<Directory*>& container) const;
        void GetFiles(std::vector<File*>& container) const;
        void CreateFile(string name) const;
        void DeleteFile(string name) const;
        void CreateDirectory(string name) const;
        void DeleteDirectory(string name) const;
        File* GetFile(string path) const;
        Directory* GetDirectory(string path) const;
        string GetName() const;
        tm* GetCreatedTime() const;
        tm* GetLastModifiedTime() const;
        Path* GetPath() const { return this->path; }

        void Move(string srcPath, string destPath);
        void Copy(string srcPath, string destPath);
        void Write() const;

        void Import(string path);
    };
}

#endif // VFAT_DIRECTORY_H
