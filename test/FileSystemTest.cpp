#include "gtest/gtest.h"
#include "../include/api/FileSystem.h"
#include "../include/api/Directory.h"

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
    ASSERT_EQ("/", rootDir->GetName());

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
