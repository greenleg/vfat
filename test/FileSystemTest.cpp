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

    // Check directories;
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

    for (size_t i = 0; i < directories.size(); i++) {
        delete directories.at(i);
    }

    directories.clear();
    dir01->GetDirectories(directories);
    ASSERT_EQ(1, directories.size());
    ASSERT_EQ("home", directories.at(0)->GetName());

    for (size_t i = 0; i < directories.size(); i++) {
        delete directories.at(i);
    }

    directories.clear();
    dir02->GetDirectories(directories);
    ASSERT_EQ(2, directories.size());

    for (size_t i = 0; i < directories.size(); i++) {
        delete directories.at(i);
    }

    delete dir02;
    delete dir01;
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

    delete file0;
    delete dir00;
    delete dir0;
    delete rootDir;

    fs.Close();

    // Check for the file content;
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

TEST_F(FileSystemTest, GetDirectory)
{
    FileSystem fs(this->device);

    fs.Open();

    // Create directories;
    Directory *rootDir = Directory::GetRoot(&fs);
    rootDir->CreateDirectory("home");
    rootDir->CreateDirectory("lib");
    rootDir->CreateDirectory("var");
    Directory *dir0 = rootDir->GetDirectory("home");
    dir0->CreateDirectory("user");
    Directory *dir00 = dir0->GetDirectory("user");
    dir00->CreateDirectory("Documents");
    dir00->CreateDirectory("Projects");
    Directory *dir001 = dir00->GetDirectory("Projects");
    Directory *dir00_copy = dir001->GetDirectory("..");

    // Check directories;
    vector<Directory*> directories;
    dir00_copy->GetDirectories(directories);
    ASSERT_EQ(4, directories.size());
    ASSERT_EQ(".", directories.at(0)->GetName());
    ASSERT_EQ("..", directories.at(1)->GetName());
    ASSERT_EQ("Documents", directories.at(2)->GetName());
    ASSERT_EQ("Projects", directories.at(3)->GetName());

    for (size_t i = 0; i < directories.size(); i++) {
        delete directories.at(i);
    }

    Directory *dir1 = dir001->GetDirectory("../../../lib"); // !
    directories.clear();
    dir1->GetDirectories(directories);
    ASSERT_EQ(2, directories.size());
    ASSERT_EQ(".", directories.at(0)->GetName());
    ASSERT_EQ("..", directories.at(1)->GetName());

    for (size_t i = 0; i < directories.size(); i++) {
        delete directories.at(i);
    }

    delete dir1;
    delete dir00_copy;
    delete dir001;
    delete dir00;
    delete dir0;
    delete rootDir;

    fs.Close();
}

TEST_F(FileSystemTest, DeleteDirectory)
{
    FileSystem fs(this->device);

    fs.Open();

    // Create directories;
    Directory *rootDir = Directory::GetRoot(&fs);
    rootDir->CreateDirectory("home");
    rootDir->CreateDirectory("lib");
    rootDir->CreateDirectory("var");
    Directory *dir0 = rootDir->GetDirectory("home");
    dir0->CreateDirectory("user");
    Directory *dir00 = dir0->GetDirectory("user");
    dir00->CreateDirectory("Documents");
    dir00->CreateDirectory("Projects");
    Directory *dir001 = dir00->GetDirectory("Projects");

    dir00->DeleteDirectory("Projects");

    bool errorWasThrown = false;
    try {
        rootDir->GetDirectory("/home/user/Projects");
    } catch (std::runtime_error error) {
        ASSERT_STREQ("Directory doesn't exist.", error.what());
        errorWasThrown = true;
    }

    ASSERT_TRUE(errorWasThrown);

    delete dir001;
    delete dir00;
    delete dir0;
    delete rootDir;

    fs.Close();
}

TEST_F(FileSystemTest, MoveDirectory)
{
    FileSystem fs(this->device);

    fs.Open();

    // Create directories;
    Directory *rootDir = Directory::GetRoot(&fs);
    rootDir->CreateDirectory("home");
    rootDir->CreateDirectory("lib");
    rootDir->CreateDirectory("var");
    Directory *dir0 = rootDir->GetDirectory("home");
    dir0->CreateDirectory("user");
    Directory *dir00 = dir0->GetDirectory("user");
    dir00->CreateDirectory("Documents");
    dir00->CreateDirectory("Projects");
    Directory *dir011 = dir00->GetDirectory("Projects");
    dir011->CreateDirectory("vfat");

    // Perform moving folder '/home/user' to '/'
    dir0->MoveFile("user", "..");

    delete dir011;
    delete dir00;
    delete dir0;
    delete rootDir;

    fs.Close();

    // Check directories;
    fs.Open();
    rootDir = Directory::GetRoot(&fs);
    dir0 = rootDir->GetDirectory("home");

    bool errorWasThrown = false;
    try {
        rootDir->GetDirectory("/home/user");
    } catch (std::runtime_error error) {
        ASSERT_STREQ("Directory doesn't exist.", error.what());
        errorWasThrown = true;
    }

    ASSERT_TRUE(errorWasThrown);

    Directory *dir110 = rootDir->GetDirectory("/user/Projects/vfat");

    delete dir0;
    delete dir110;

    fs.Close();
}

TEST_F(FileSystemTest, MoveFile)
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
    file0->WriteText("A journey of thousand miles begins with a single step.", 0);

    // Re-read 'home' directory.
    Directory *dir0_copy = rootDir->GetDirectory("home");
    dir0_copy->MoveFile("./user/dump0.bin", "..");

    delete dir0_copy;
    delete file0;
    delete dir00;
    delete dir0;
    delete rootDir;

    fs.Close();

    // Check that the file was properly moved;
    fs.Open();

    rootDir = Directory::GetRoot(&fs);
    file0 = rootDir->GetFile("/dump0.bin");
    string text = file0->ReadText(0, file0->GetSize());
    ASSERT_EQ("A journey of thousand miles begins with a single step.", text);

    delete file0;
    delete rootDir;

    fs.Close();
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
