#include "gtest/gtest.h"
#include "../include/ClusterChain.h"
#include "../include/ClusterChainDirectory.h"
#include "../include/DirectoryEntry.h"

using namespace org::vfat;

class ClusterChainDirectoryTest : public ::testing::Test
{
protected:
    FileDisk *device;

    void SetUp() override
    {
        this->device = new FileDisk("disk0");
        this->device->Create();
        ClusterChainDirectory::FormatDevice(this->device, 1024 * 1024, 512, 1);
    }

    void TearDown() override
    {
        this->device->Close();
        this->device->Delete();
        delete this->device;
    }
};

TEST_F(ClusterChainDirectoryTest, AddEntry)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    ClusterChainDirectory *root = new ClusterChainDirectory();
    root->ReadRoot(this->device, &fat);

    ASSERT_EQ(0, root->GetEntries()->size());

    DirectoryEntry *e = new DirectoryEntry();
    root->AddEntry(e);
    ASSERT_EQ(1, root->GetEntries()->size());
    ASSERT_NE(0, bootSector.GetRootDirFirstCluster());
    //ASSERT_EQ(root.capacity, root->chain->GetSizeInBytes() / FAT_DIR_ENTRY_SIZE);

//    delete e;
    delete root;
}

TEST_F(ClusterChainDirectoryTest, AddRemoveEntries)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    ClusterChainDirectory *root = new ClusterChainDirectory();
    root->ReadRoot(this->device, &fat);

    ASSERT_EQ(0, root->GetEntries()->size());

    for (uint32_t i = 0; i < 100; i++) {
        DirectoryEntry *e = new DirectoryEntry();
        root->AddEntry(e);
    }

    for (uint32_t i = 0; i < 100; i++) {
        ASSERT_EQ(100 - i, root->GetEntries()->size());
        root->RemoveEntry(0);
    }

    ASSERT_EQ(0, root->GetEntries()->size());

    delete root;
}

TEST_F(ClusterChainDirectoryTest, AddSubDirectory)
{
    BootSector bootSector;    
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    ClusterChainDirectory *root = new ClusterChainDirectory();
    root->ReadRoot(this->device, &fat);

    const char *name = "A nice directory";

    DirectoryEntry *e1 = root->AddDirectory(name, this->device);
    DirectoryEntry *e2 = root->FindEntry(name);

    char nameBuf[32];
    e1->GetName(nameBuf);
    ASSERT_STREQ(name, nameBuf);

    e2->GetName(nameBuf);
    ASSERT_STREQ(name, nameBuf);

    delete root;
}

TEST_F(ClusterChainDirectoryTest, AddTooManyDirectories)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    ClusterChainDirectory *root = new ClusterChainDirectory();
    root->ReadRoot(this->device, &fat);

    char nameBuf[255];
    uint32_t count = 0;

    do {
        uint32_t freeBeforeAdd = fat.GetFreeClusterCount();
        sprintf(nameBuf, "this is test directory with index %d", count++);
        //bool errorWasThrown = false;
        try {
            DirectoryEntry *e = root->AddDirectory(nameBuf, this->device);
        } catch (std::runtime_error err) {
            ASSERT_EQ(freeBeforeAdd, fat.GetFreeClusterCount());
            break;
        }
    } while (true);

    delete root;
}

TEST_F(ClusterChainDirectoryTest, RemoveDirectory)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    ClusterChainDirectory *root = new ClusterChainDirectory();
    root->ReadRoot(this->device, &fat);

    uint32_t freeBefore = fat.GetFreeClusterCount();
    uint32_t entriesBefore = root->GetEntries()->size();

    const char *dirName = "testdir";
    root->AddDirectory(dirName, this->device);
    ASSERT_EQ(freeBefore - 1, fat.GetFreeClusterCount());
    ASSERT_EQ(entriesBefore + 1, root->GetEntries()->size());
    ASSERT_NE(root->FindEntry(dirName), nullptr);

    root->RemoveDirectory(dirName, this->device);
    ASSERT_EQ(freeBefore, fat.GetFreeClusterCount());
    ASSERT_EQ(entriesBefore, root->GetEntries()->size());
    ASSERT_FALSE(root->FindEntry(dirName));

    delete root;
}

TEST_F(ClusterChainDirectoryTest, UniqueDirectoryName)
{
    BootSector bootSector;    
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    ClusterChainDirectory *root = new ClusterChainDirectory();
    root->ReadRoot(this->device, &fat);

    ASSERT_TRUE(root->AddDirectory("home", this->device));
    bool errorWasThrown = false;
    try {
        DirectoryEntry *e = root->AddDirectory("home", this->device);
    } catch (std::runtime_error error) {
        errorWasThrown = true;
    }

    ASSERT_TRUE(errorWasThrown);

    // Reuse name
    root->RemoveDirectory("home", this->device);
    ASSERT_NE(root->AddDirectory("home", this->device), nullptr);

    delete root;
}

TEST_F(ClusterChainDirectoryTest, RenameFile)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    ClusterChainDirectory *root = new ClusterChainDirectory();
    root->ReadRoot(this->device, &fat);

    DirectoryEntry *e = root->AddFile("oldfile", this->device);

    ASSERT_NE(root->FindEntry("oldfile"), nullptr);
    ASSERT_EQ(root->FindEntry("newfile"), nullptr);

    root->SetName(this->device, e, "newfile");

    ASSERT_EQ(root->FindEntry("oldfile"), nullptr);
    ASSERT_NE(root->FindEntry("newfile"), nullptr);

    delete root;
}

TEST_F(ClusterChainDirectoryTest, MoveFile)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    ClusterChainDirectory *root = new ClusterChainDirectory();
    root->ReadRoot(this->device, &fat);

    DirectoryEntry *de = root->AddDirectory("home", this->device);
    DirectoryEntry *fe = root->AddFile("dump.bin", this->device);

    ASSERT_NE(de, nullptr);
    ASSERT_NE(fe, nullptr);
    ClusterChainDirectory *dir = ClusterChainDirectory::GetDirectory(this->device, &fat, de);
    root->Move(this->device, fe, dir, "dump2.bin");    

    ASSERT_EQ(1, root->GetEntries()->size());
    ASSERT_EQ(3, dir->GetEntries()->size());  // including "." and ".." directories.    
    ASSERT_NE(root->FindEntry("home"), nullptr);
    ASSERT_EQ(root->FindEntry("dump.bin"), nullptr);
    ASSERT_EQ(root->FindEntry("dump2.bin"), nullptr);
    ASSERT_NE(dir->FindEntry("dump2.bin"), nullptr);

    delete dir;
    delete root;
}

TEST_F(ClusterChainDirectoryTest, MoveDirectory)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    ClusterChainDirectory *root = new ClusterChainDirectory();
    root->ReadRoot(this->device, &fat);

    DirectoryEntry *de1 = root->AddDirectory("dir1", this->device);
    DirectoryEntry *de2 = root->AddDirectory("dir2", this->device);

    ClusterChainDirectory *dir1 = ClusterChainDirectory::GetDirectory(this->device, &fat, de1);
    ClusterChainDirectory *dir2 = ClusterChainDirectory::GetDirectory(this->device, &fat, de2);

    DirectoryEntry *fe1 = dir1->AddFile("dump1.bin", this->device);
    DirectoryEntry *fe2 = dir1->AddFile("dump2.bin", this->device);
    DirectoryEntry *fe3 = dir1->AddFile("dump3.bin", this->device);

    ClusterChainFile *file3 = ClusterChainDirectory::GetFile(&fat, fe3);

    ASSERT_EQ(2, root->GetEntries()->size());
    ASSERT_EQ(5, dir1->GetEntries()->size()); // including "." and ".." directories
    ASSERT_EQ(2, dir2->GetEntries()->size());

    root->Move(this->device, de1, dir2, "dir1");

    dir1->Write(this->device);
    dir2->Write(this->device);

    delete dir1;
    delete dir2;

    ASSERT_NE(root->FindEntry("dir2"), nullptr);
    dir2 = ClusterChainDirectory::GetDirectory(this->device, &fat, de2);

    ASSERT_NE(dir2->FindEntry("dir1"), nullptr);
    dir1 = ClusterChainDirectory::GetDirectory(this->device, &fat, de1);

    ASSERT_EQ(1, root->GetEntries()->size());
    ASSERT_EQ(5, dir1->GetEntries()->size());
    ASSERT_EQ(3, dir2->GetEntries()->size());

    delete file3;
    delete dir1;
    delete dir2;
    delete root;
}

TEST_F(ClusterChainDirectoryTest, CopyFile)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    ClusterChainDirectory *root = new ClusterChainDirectory();
    root->ReadRoot(this->device, &fat);

    DirectoryEntry *de1 = root->AddDirectory("dir1", this->device);
    DirectoryEntry *de2 = root->AddDirectory("dir2", this->device);

    ClusterChainDirectory *dir1 = ClusterChainDirectory::GetDirectory(this->device, &fat, de1);
    ClusterChainDirectory *dir2 = ClusterChainDirectory::GetDirectory(this->device, &fat, de2);
    DirectoryEntry *orige = dir1->AddFile("dump.bin", this->device);

    ASSERT_EQ(2, root->GetEntries()->size());
    ASSERT_EQ(3, dir1->GetEntries()->size()); // including "." and ".." directories
    ASSERT_EQ(2, dir2->GetEntries()->size());

    uint32_t len = 4096 * 4;
    uint8_t buf[len];
    for (uint32_t i = 0; i < len; i++) {
        buf[i] = i % 256;
    }    

    // Write to source file    
    ClusterChainFile *orig = ClusterChainDirectory::GetFile(&fat, orige);
    orig->Write(this->device, 0, len, buf);

    dir1->CopyFile(this->device, orige, dir2);

    orige = dir1->FindEntry("dump.bin");
    DirectoryEntry *copye = dir2->FindEntry("dump.bin");
    ASSERT_NE(orige, nullptr);
    ASSERT_NE(copye, nullptr);

    // Read from copy
    ClusterChainFile *copy = ClusterChainDirectory::GetFile(&fat, copye);
    ASSERT_EQ(len, copy->GetLength());
    uint32_t nread = copy->Read(this->device, 0, len, buf);

    for (uint32_t i = 0; i < len; i++) {
        ASSERT_EQ(i % 256, buf[i]);
    }

    ASSERT_EQ(2, root->GetEntries()->size());
    ASSERT_EQ(3, dir1->GetEntries()->size());
    ASSERT_EQ(3, dir2->GetEntries()->size());

    delete orig;
    delete copy;
    delete dir1;
    delete dir2;
    delete root;
}

TEST_F(ClusterChainDirectoryTest, CopyDirectory)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    ClusterChainDirectory *root = new ClusterChainDirectory();
    root->ReadRoot(this->device, &fat);

    DirectoryEntry *de1 = root->AddDirectory("dir1", this->device);

    DirectoryEntry *de2 = root->AddDirectory("dir2", this->device);
    ClusterChainDirectory *dir1 = ClusterChainDirectory::GetDirectory(this->device, &fat, de1);
    ClusterChainDirectory *dir2 = ClusterChainDirectory::GetDirectory(this->device, &fat, de2);

    //EXPECT_TRUE(cchdir_addfile(&dir1, "dump.bin", &fe));
    DirectoryEntry *fe = dir1->AddFile("dump.bin", this->device);

    // Keep in mind that all directories except root include "." and ".." sub-directories
    ASSERT_EQ(2, root->GetEntries()->size());
    ASSERT_EQ(3, dir1->GetEntries()->size());
    ASSERT_EQ(2, dir2->GetEntries()->size());

    uint32_t len = 4096 * 4 + 100;
    uint8_t buf[len];
    for (uint32_t i = 0; i < len; i++) {
        buf[i] = i % 256;
    }

    // Write to file
    ClusterChainFile *file = ClusterChainDirectory::GetFile(&fat, fe);
    file->Write(this->device, 0, len, buf);

    dir1->Write(this->device);
    dir2->Write(this->device);

    root->CopyDirectory(this->device, de1, dir2);

    DirectoryEntry *copyde1 = dir2->FindEntry("dir1");
    ClusterChainFile *copyfile;

    ASSERT_NE(root->FindEntry("dir1"), nullptr);
    ASSERT_NE(root->FindEntry("dir2"), nullptr);
    ASSERT_NE(copyde1, nullptr);
    ASSERT_NE(dir1->FindEntry("dump.bin"), nullptr);

    ClusterChainDirectory *copydir1 = ClusterChainDirectory::GetDirectory(this->device, &fat, copyde1);
    DirectoryEntry *copyfe = copydir1->FindEntry("dump.bin");
    ASSERT_NE(nullptr, copyfe);

    ASSERT_EQ(2, root->GetEntries()->size());
    ASSERT_EQ(3, dir1->GetEntries()->size());
    ASSERT_EQ(3, dir2->GetEntries()->size());
    ASSERT_EQ(3, copydir1->GetEntries()->size());

    // Read from copy
    copyfile = ClusterChainDirectory::GetFile(&fat, copyfe);
    ASSERT_EQ(len, copyfile->GetLength());
    uint32_t nread = copyfile->Read(this->device, 0, len, buf);

    for (uint32_t i = 0; i < len; i++) {
        ASSERT_EQ(i % 256, buf[i]);
    }

    // Writing to the copy doesn't affect to the origin file.
    buf[0] = 50;
    copyfile->Write(this->device, 0, 1, buf);
    nread = file->Read(this->device, 0, 1, buf);
    ASSERT_EQ(0, buf[0]);
    nread = copyfile->Read(this->device, 0, 1, buf);
    ASSERT_EQ(50, buf[0]);

    delete copyfile;
    delete file;

    delete copydir1;
    delete dir1;
    delete dir2;
    delete root;
}
