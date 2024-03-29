#ifndef VFAT_DIRECTORY_H
#define VFAT_DIRECTORY_H

#include <string>
#include "../ClusterChainDirectory.h"
#include "DirectoryItem.h"
#include "FileSystem.h"
#include "File.h"
#include "Path.h"

using namespace org::vfat;

namespace org::vfat::api
{
    class Directory : public DirectoryItem
    {
    private:
        FileSystem *fs;
        ClusterChainDirectory parentCchDir;
        DirectoryEntry rootEntry;
        Path path;

        bool IsRoot() const;
        void Move(ClusterChainDirectory& srcDir, DirectoryEntry& srcEntry, ClusterChainDirectory& destDir, const std::string& destName);
        void CopyFile(ClusterChainDirectory& srcDir, DirectoryEntry& srcEntry, ClusterChainDirectory& destDir, const std::string& destName);
        void CopyDirectory(ClusterChainDirectory& srcDir, DirectoryEntry& srcEntry, ClusterChainDirectory& destDir, const std::string& destName);
        void ImportFile(const std::string& path);
        void ImportDirectory(const std::string& path);
        ClusterChainDirectory GetCchDirectory() const;
        void Init();
        void Cleanup();
        const DirectoryEntry& GetThisEntry() const;

    public:
//        ClusterChainDirectory* GetParentCchDirectory() const {  return this->parentCchDir; }
        Directory();
        Directory(FileSystem *fs, Path& path);
        Directory(FileSystem *fs, Path&& path);

        Directory(const Directory& other);
        Directory(Directory&& other);
        Directory& operator=(const Directory& other);
        Directory& operator=(Directory&& other);

        static Directory GetRoot(FileSystem *fs);
        ~Directory();
        std::vector<Directory> GetDirectories() const;
        std::vector<File> GetFiles() const;

        void CreateFile(const std::string& name) const;
        void DeleteFile(const std::string& path) const;
        void CreateDirectory(const std::string& name) const;
        void DeleteDirectory(const std::string& path) const;
        File GetFile(const std::string& path) const;
        Directory GetDirectory(const std::string& path) const;        
        Path GetPath() const { return this->path; }

        void Move(const std::string& srcPath, const std::string& destPath);
        void Copy(const std::string& srcPath, const std::string& destPath);
        void Write() const;

        void Import(const std::string& path);

        std::string GetName() const;
        tm* GetCreatedTime() const;
        tm* GetModifiedTime() const;
    };
}

#endif // VFAT_DIRECTORY_H
