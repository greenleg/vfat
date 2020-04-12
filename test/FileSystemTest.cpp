#include <cstring>
#include "gtest/gtest.h"
#include "../include/api/FileSystem.h"
#include "../include/api/Directory.h"
#include "../include/Common.h"

using namespace org::vfat;
using namespace org::vfat::api;

class FileSystemTest : public ::testing::Test
{
protected:
    FileDisk *device;

    void SetUp() override
    {
        this->device = new FileDisk("disk0");
        this->device->Create();

        FileSystem fs(this->device);
        fs.Format(1024 * 1024, 512, 1);
        fs.Close();
    }

    void TearDown() override
    {
        this->device->Close();
        this->device->Delete();
        delete this->device;
    }

//    static void PrintDirectory(struct filesys *fs, struct vdir *dir, int level)
//    {
//        struct vdir *subdir;
//        struct vdirent e;
//        while(filesys_readdir(dir, &e)) {
//            if (strcmp(".", e.name) == 0 || strcmp("..", e.name) == 0) {
//                continue;
//            }

//            for (int i = 0; i < level; ++i) {
//                printf("*\t");
//            }

//            printf("%s\n", e.name);

//            subdir = filesys_getdir(fs, dir, e.name);
//            PrintDirectory(fs, subdir, level + 1);
//            filesys_closedir(fs, subdir);
//        }
//    }
//    static void PrintDirectory()
//    {

//    }
};

TEST_F(FileSystemTest, MakeDirectory)
{
    FileSystem fs(this->device);

    // Create directories
    fs.Open();

    Directory *rootDir = Directory::GetRoot(&fs);
    rootDir->CreateDirectory("home");
    Directory *dir0 = rootDir->GetDirectory("home");
    dir0->CreateDirectory("user");

    delete dir0;
    delete rootDir;

    fs.Close();

    // Check directories
    fs.Open();

    rootDir = Directory::GetRoot(&fs);
    ASSERT_EQ("/", rootDir->GetName());

    vector<Directory*> directories;
    rootDir->GetDirectories(directories);
    ASSERT_EQ(1, directories.size());
    dir0 = directories.at(0);
    ASSERT_EQ("home", dir0->GetName());

    directories.clear();
    dir0->GetDirectories(directories);
    ASSERT_EQ(3, directories.size());
    Directory *dir00 = directories.at(0);
    ASSERT_EQ(".", dir00->GetName());
    Directory *dir01 = directories.at(1);
    ASSERT_EQ("..", dir01->GetName());
    Directory *dir02 = directories.at(2);
    ASSERT_EQ("user", dir02->GetName());

    directories.clear();
    dir00->GetDirectories(directories);
    ASSERT_EQ(3, directories.size());

    directories.clear();
    dir01->GetDirectories(directories);
    ASSERT_EQ(1, directories.size());
    Directory *dir011 = directories.at(0);
    ASSERT_EQ("home", dir011->GetName());

    directories.clear();
    dir02->GetDirectories(directories);
    ASSERT_EQ(2, directories.size());

    delete dir00;
    delete dir0;
    delete rootDir;

    fs.Close();
}

TEST_F(FileSystemTest, CreateFile)
{
    FileSystem fs(this->device);

    // Create directories;
    fs.Open();

    Directory *rootDir = Directory::GetRoot(&fs);
    rootDir->CreateDirectory("home");
    Directory *dir0 = rootDir->GetDirectory("home");
    dir0->CreateDirectory("user");
    Directory *dir00 = dir0->GetDirectory("user");

    // Create a file and write data to it;
    dir00->CreateFile("dump0.bin");
    File *file0 = dir00->GetFile("dump0.bin");
    file0->WriteText("The quick brown fox jumps over the lazy dog.", 0);

    // The parent directory contains information about a file including name, size, creation time etc.
    // This updated information should be stored to a device as well.
    dir00->Write();

    delete file0;
    delete dir00;
    delete dir0;
    delete rootDir;

    fs.Close();

    // Check for a file content;
    fs.Open();

    rootDir = Directory::GetRoot(&fs);
    dir0 = rootDir->GetDirectory("home");
    dir00 = dir0->GetDirectory("user");

    vector<File*> files;
    dir00->GetFiles(files);
    ASSERT_EQ(1, files.size());
    file0 = files.at(0);
    ASSERT_EQ("dump0.bin", file0->GetName());

    string s = file0->ReadText(0, file0->GetSize());
    ASSERT_EQ("The quick brown fox jumps over the lazy dog.", s);

    File *file0_copy = dir00->GetFile("dump0.bin");
    string s_copy = file0->ReadText(0, file0_copy->GetSize());
    ASSERT_EQ("The quick brown fox jumps over the lazy dog.", s_copy);

    delete file0_copy;
    delete file0;
    delete dir00;
    delete dir0;
    delete rootDir;

    fs.Close();
}

TEST_F(FileSystemTest, DeleteDirectory)
{

}

//TEST_F(FileSystemTest, ReadDirectory)
//{
//    struct filesys fs;

//    filesys_open(this->device, &fs);

//    filesys_mkdir(&fs, "/home/pavel/projects/vfat");
//    filesys_mkdir(&fs, "/home/pavel/projects/old");
//    filesys_mkdir(&fs, "/home/pavel/Desktop");
//    filesys_mkdir(&fs, "/home/pavel/Documents");
//    filesys_mkdir(&fs, "/home/pavel/Downloads");
//    filesys_mkdir(&fs, "/home/pavel/Downloads/astyle");
//    filesys_mkdir(&fs, "/home/pavel/Qt");
//    filesys_mkdir(&fs, "/home/pavel/Qt/Docs");
//    filesys_mkdir(&fs, "/home/pavel/Qt/Examples");
//    filesys_mkdir(&fs, "/home/pavel/Qt/Tools");

//    filesys_close(&fs);
//    filesys_destruct(&fs);

//    filesys_open(this->device, &fs);

//    struct vdir *dir = filesys_opendir(&fs, "/");
//    this->PrintDirectory(&fs, dir, 0);

//    filesys_closedir(&fs, dir);
//    filesys_close(&fs);
//    filesys_destruct(&fs);
//}
