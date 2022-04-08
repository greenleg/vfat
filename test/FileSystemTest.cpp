#include <cstring>
#include <iostream>
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
        fs.Write();
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
    {
        FileSystem fs(this->device);
        fs.Read();

        // Create directories;
        Directory *rootDir = Directory::GetRoot(&fs);
        rootDir->CreateDirectory("home");
        Directory *dir0 = rootDir->GetDirectory("home");
        dir0->CreateDirectory("user");

        delete dir0;
        delete rootDir;

        fs.Write();
    }

    {
        FileSystem fs(this->device);
        fs.Read();

        // Check directories;
        Directory *rootDir = Directory::GetRoot(&fs);
        ASSERT_EQ("/", rootDir->GetName());

        vector<Directory*> directories;
        rootDir->GetDirectories(directories);
        ASSERT_EQ(1, directories.size());
        Directory *dir0 = directories.at(0);
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

        fs.Write();
    }
}

TEST_F(FileSystemTest, MakeDirectory2)
{
    FileSystem fs(this->device);
    fs.Read();

    // Create directories;
    Directory *rootDir = Directory::GetRoot(&fs);
    rootDir->CreateDirectory("home");
    Directory *dir0 = rootDir->GetDirectory("home");
    Directory* rootDir2 = dir0->GetDirectory("..");

    // Check Create/Modified time;
    ASSERT_EQ(Utils::FormatDate(rootDir->GetCreatedTime()),
              Utils::FormatDate(rootDir2->GetCreatedTime()));

    ASSERT_EQ(Utils::FormatDate(rootDir->GetModifiedTime()),
              Utils::FormatDate(rootDir2->GetModifiedTime()));

    delete dir0;
    delete rootDir;
    delete rootDir2;
}

TEST_F(FileSystemTest, CreateFile)
{
    {
        FileSystem fs(this->device);
        fs.Read();

        // Create directories;
        Directory *rootDir = Directory::GetRoot(&fs);
        rootDir->CreateDirectory("home");
        Directory *dir0 = rootDir->GetDirectory("home");
        dir0->CreateDirectory("user");
        Directory *dir00 = dir0->GetDirectory("user");

        // Create a file and write data to it;
        dir00->CreateFile("dump0.bin");
        File file0 = dir00->GetFile("dump0.bin");
        file0.WriteText("The quick brown fox jumps over the lazy dog.", 0);

        delete dir00;
        delete dir0;
        delete rootDir;

        fs.Write();
    }

    {
        // Check for the file content;
        FileSystem fs(this->device);
        fs.Read();

        Directory *rootDir = Directory::GetRoot(&fs);
        Directory *dir0 = rootDir->GetDirectory("home");
        Directory *dir00 = dir0->GetDirectory("user");

        vector<File*> files;
        dir00->GetFiles(files);
        ASSERT_EQ(1, files.size());
        File *file0 = files.at(0);
        ASSERT_EQ("dump0.bin", file0->GetName());

        std::string s = file0->ReadText(0, file0->GetSize());
        ASSERT_EQ("The quick brown fox jumps over the lazy dog.", s);

        File file0_copy = dir00->GetFile("dump0.bin");
        std::string s_copy = file0->ReadText(0, file0_copy.GetSize());
        ASSERT_EQ("The quick brown fox jumps over the lazy dog.", s_copy);

        delete file0;
        delete dir00;
        delete dir0;
        delete rootDir;

        fs.Write();
    }
}

TEST_F(FileSystemTest, GetDirectory)
{
    FileSystem fs(this->device);
    fs.Read();

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

    fs.Write();
}

TEST_F(FileSystemTest, DeleteDirectory)
{
    FileSystem fs(this->device);
    fs.Read();

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
    } catch (const std::ios_base::failure& error) {
        ASSERT_STREQ("Couldn't find '/home/user/Projects': No such file or directory: iostream error", error.what());
        errorWasThrown = true;
    }

    ASSERT_TRUE(errorWasThrown);

    delete dir001;
    delete dir00;
    delete dir0;
    delete rootDir;

    fs.Write();
}

TEST_F(FileSystemTest, MoveDirectory)
{
    {
        FileSystem fs(this->device);
        fs.Read();

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
        dir0->Move("user", "..");

        delete dir011;
        delete dir00;
        delete dir0;
        delete rootDir;

        fs.Write();
    }

    {
        // Check directories;
        FileSystem fs(this->device);
        fs.Read();

        Directory *rootDir = Directory::GetRoot(&fs);
        Directory *dir0 = rootDir->GetDirectory("home");

        bool errorWasThrown = false;
        try {
            rootDir->GetDirectory("/home/user");
        } catch (std::runtime_error error) {
            ASSERT_STREQ("Couldn't find '/home/user': No such file or directory: iostream error", error.what());
            errorWasThrown = true;
        }

        ASSERT_TRUE(errorWasThrown);

        Directory *dir110 = rootDir->GetDirectory("/user/Projects/vfat");

        delete dir0;
        delete dir110;

        fs.Write();
    }
}

TEST_F(FileSystemTest, MoveFile)
{
    {
        FileSystem fs(this->device);
        fs.Read();

        // Create directories;
        Directory *rootDir = Directory::GetRoot(&fs);
        rootDir->CreateDirectory("home");
        Directory *dir0 = rootDir->GetDirectory("home");
        dir0->CreateDirectory("user");
        Directory *dir00 = dir0->GetDirectory("user");

        // Create a file and write data to it;
        dir00->CreateFile("dump0.bin");
        File file0 = dir00->GetFile("dump0.bin");
        file0.WriteText("A journey of thousand miles begins with a single step.", 0);

        // Re-read 'home' directory.
        Directory *dir0_copy = rootDir->GetDirectory("home");
        dir0_copy->Move("./user/dump0.bin", "..");

        delete dir0_copy;
        delete dir00;
        delete dir0;
        delete rootDir;

        fs.Write();
    }

    {
        // Check that the file was properly moved;
        FileSystem fs(this->device);
        fs.Read();

        Directory *rootDir = Directory::GetRoot(&fs);
        File file0 = rootDir->GetFile("/dump0.bin");
        std::string text = file0.ReadText(0, file0.GetSize());
        ASSERT_EQ("A journey of thousand miles begins with a single step.", text);

        delete rootDir;

        fs.Write();
    }
}

TEST_F(FileSystemTest, CopyFile)
{
    {
        FileSystem fs(this->device);
        fs.Read();

        // Create directories;
        Directory *rootDir = Directory::GetRoot(&fs);
        rootDir->CreateDirectory("home");
        Directory *dir0 = rootDir->GetDirectory("home");
        dir0->CreateDirectory("user");
        Directory *dir00 = dir0->GetDirectory("user");

        // Create a file and write data to it;
        dir00->CreateFile("dump0.bin");
        File file0 = dir00->GetFile("dump0.bin");
        file0.WriteText("If you want something done right, you have to do it yourself.", 0);

        // Re-read 'home' directory.
        Directory *dir0_copy = rootDir->GetDirectory("home");
        dir0_copy->Copy("./user/dump0.bin", "..");
        dir0_copy->Copy("./user/dump0.bin", "../dump0_copy.bin");

        delete dir0_copy;
        delete dir00;
        delete dir0;
        delete rootDir;

        fs.Write();
    }

    {
        // Check that the file was properly copied;
        FileSystem fs(this->device);
        fs.Read();

        Directory *rootDir = Directory::GetRoot(&fs);

        File file0 = rootDir->GetFile("/home/user/dump0.bin");
        std::string text = file0.ReadText(0, file0.GetSize());
        ASSERT_EQ("If you want something done right, you have to do it yourself.", text);

        file0 = rootDir->GetFile("/dump0.bin");
        text = file0.ReadText(0, file0.GetSize());
        ASSERT_EQ("If you want something done right, you have to do it yourself.", text);

        file0 = rootDir->GetFile("/dump0_copy.bin");
        text = file0.ReadText(0, file0.GetSize());
        ASSERT_EQ("If you want something done right, you have to do it yourself.", text);

        delete rootDir;

        fs.Write();
    }
}

TEST_F(FileSystemTest, CopyDirectory)
{
    {
        FileSystem fs(this->device);
        fs.Read();

        // Create directories;
        Directory *rootDir = Directory::GetRoot(&fs);
        rootDir->CreateDirectory("home");
        Directory *dir0 = rootDir->GetDirectory("home");
        dir0->CreateDirectory("user");
        Directory *dir00 = dir0->GetDirectory("user");

        // Create a file and write data to it;
        dir00->CreateFile("dump0.bin");
        File file0 = dir00->GetFile("dump0.bin");
        file0.WriteText("Fortune favors the bold.", 0);

        // Re-read 'home' directory.
        Directory *dir0_copy = rootDir->GetDirectory("home");
        dir0_copy->Copy("./user/", "..");

        delete dir0_copy;
        delete dir00;
        delete dir0;
        delete rootDir;

        fs.Write();
    }

    {
        // Check that the file was properly copied;
        FileSystem fs(this->device);
        fs.Read();

        Directory *rootDir = Directory::GetRoot(&fs);

        File file0 = rootDir->GetFile("/home/user/dump0.bin");
        std::string text = file0.ReadText(0, file0.GetSize());
        ASSERT_EQ("Fortune favors the bold.", text);

        file0 = rootDir->GetFile("/user/dump0.bin");
        text = file0.ReadText(0, file0.GetSize());
        ASSERT_EQ("Fortune favors the bold.", text);

        delete rootDir;

        fs.Write();
    }
}

TEST_F(FileSystemTest, DeleteDirectory2)
{
    {
        FileSystem fs(this->device);
        fs.Read();

        // Create directories;
        Directory *rootDir = Directory::GetRoot(&fs);
        rootDir->CreateDirectory("home");
        Directory *dir0 = rootDir->GetDirectory("home");
        dir0->CreateDirectory("user");
        Directory *dir00 = dir0->GetDirectory("user");

        // Create a file and write data to it;
        dir0->CreateFile("file0.txt");
        File file0 = dir0->GetFile("file0.txt");
        file0.WriteText("Actions speak louder than words.", 0);

        // Create a file and write data to it;
        dir00->CreateFile("file1.txt");
        File file1 = dir00->GetFile("file1.txt");
        file1.WriteText("An idle brain is the devil’s workshop.", 0);

        // Remove a directory;
        rootDir->DeleteDirectory("home/user");

        delete dir00;
        delete dir0;
        delete rootDir;

        fs.Write();
    }

    {
        FileSystem fs(this->device);
        fs.Read();

        Directory *rootDir = Directory::GetRoot(&fs);

        bool errorWasThrown = false;
        try {
            rootDir->GetDirectory("home/user");
        } catch (const std::ios_base::failure& err) {
            errorWasThrown = true;
        }

        ASSERT_TRUE(errorWasThrown);

        File file0 = rootDir->GetFile("/home/file0.txt");
        ASSERT_EQ("Actions speak louder than words.", file0.ReadText(0, file0.GetSize()));

        delete rootDir;
    }
}

TEST_F(FileSystemTest, DeleteFile)
{
    {
        FileSystem fs(this->device);
        fs.Read();

        // Create directories;
        Directory *rootDir = Directory::GetRoot(&fs);
        rootDir->CreateDirectory("home");
        Directory *dir0 = rootDir->GetDirectory("home");
        dir0->CreateDirectory("user");
        Directory *dir00 = dir0->GetDirectory("user");

        // Create a file and write data to it;
        dir0->CreateFile("file0.txt");
        File file0 = dir0->GetFile("file0.txt");
        file0.WriteText("Actions speak louder than words.", 0);

        // Create a file and write data to it;
        dir00->CreateFile("file1.txt");
        File file1 = dir00->GetFile("file1.txt");
        file1.WriteText("An idle brain is the devil’s workshop.", 0);

        // Remove all files;
        rootDir->DeleteFile("home/file0.txt");
        rootDir->DeleteFile("home/user/file1.txt");

        delete dir00;
        delete dir0;
        delete rootDir;

        fs.Write();
    }

    {
        FileSystem fs(this->device);
        fs.Read();

        Directory *rootDir = Directory::GetRoot(&fs);
        Directory *dir0 = rootDir->GetDirectory("home");
        Directory *dir00 = rootDir->GetDirectory("home/user");

        bool errorWasThrown = false;
        try {
            rootDir->GetFile("/home/file0.txt");
        } catch (const std::ios_base::failure& err) {
            errorWasThrown = true;
            ASSERT_STREQ("Couldn't find '/home/file0.txt': No such file or directory: iostream error", err.what());
        }

        ASSERT_TRUE(errorWasThrown);

        errorWasThrown = false;
        try {
            rootDir->GetFile("/home/user/file1.txt");
        } catch (const std::ios_base::failure& err) {
            errorWasThrown = true;
            ASSERT_STREQ("Couldn't find '/home/user/file1.txt': No such file or directory: iostream error", err.what());
        }

        ASSERT_TRUE(errorWasThrown);

        delete dir00;
        delete dir0;
        delete rootDir;
    }
}
