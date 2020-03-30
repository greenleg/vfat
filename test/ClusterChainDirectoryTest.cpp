#include "gtest/gtest.h"
#include "../include/common.h"
#include "../include/alist.h"
#include "../include/ClusterChain.h"
#include "../include/cchdir.h"
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
        ::__vfat_errno = 0;
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

    delete e;
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
        } catch (std::runtime_error error) {
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

    ASSERT_TRUE(root->RemoveDirectory(dirName));
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
    ASSERT_TRUE(root->RemoveDirectory("home"));
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

    DirectoryEntry *e = root->AddFile("oldfile");

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

    struct cchdir root;
    cchdir_readroot(this->device, &fat, &root);

    DirectoryEntry fe;
    DirectoryEntry de;
    struct cchdir dir;

    EXPECT_TRUE(cchdir_adddir(&root, "home", &de, &dir));
    EXPECT_TRUE(cchdir_addfile(&root, "dump.bin", &fe));

    EXPECT_TRUE(cchdir_move(this->device, &root, &fe, &dir, "dump2.bin"));

    EXPECT_EQ(1, alist_count(root.entries));
    EXPECT_EQ(3, alist_count(dir.entries));  // including "." and ".." directories.
    EXPECT_TRUE(cchdir_findentry(&root, "home", &de));
    EXPECT_FALSE(cchdir_findentry(&root, "dump2.bin", &de));
    EXPECT_TRUE(cchdir_findentry(&dir, "dump2.bin", &de));

    cchdir_destruct(&dir);
    cchdir_destruct(&root);
}

TEST_F(ClusterChainDirectoryTest, MoveDirectory)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    struct cchdir root;
    cchdir_readroot(this->device, &fat, &root);

    DirectoryEntry fe1;
    DirectoryEntry fe2;
    DirectoryEntry fe3;

    DirectoryEntry de1;
    DirectoryEntry de2;

    struct cchdir dir1;
    struct cchdir dir2;

    struct cchfile file3;

    EXPECT_TRUE(cchdir_adddir(&root, "dir1", &de1, &dir1));
    cchdir_write(&dir1, this->device);

    EXPECT_TRUE(cchdir_adddir(&root, "dir2", &de2, &dir2));
    cchdir_write(&dir2, this->device);

    EXPECT_TRUE(cchdir_addfile(&dir1, "dump1.bin", &fe1));
    EXPECT_TRUE(cchdir_addfile(&dir1, "dump2.bin", &fe2));
    EXPECT_TRUE(cchdir_addfile(&dir1, "dump3.bin", &fe3));

    cchdir_getfile(&dir1, &fe3, &file3);

    EXPECT_EQ(2, alist_count(root.entries));
    EXPECT_EQ(5, alist_count(dir1.entries)); // including "." and ".." directories
    EXPECT_EQ(2, alist_count(dir2.entries));

    EXPECT_TRUE(cchdir_move(this->device, &root, &de1, &dir2, "dir1"));

    cchdir_write(&dir1, this->device);
    cchdir_write(&dir2, this->device);

    cchdir_destruct(&dir1);
    cchdir_destruct(&dir2);

    EXPECT_TRUE(cchdir_findentry(&root, "dir2", &de2));
    cchdir_getdir(this->device, &fat, &de2, &dir2);

    EXPECT_TRUE(cchdir_findentry(&dir2, "dir1", &de1));
    cchdir_getdir(this->device, &fat, &de1, &dir1);

    EXPECT_EQ(1, alist_count(root.entries));
    EXPECT_EQ(5, alist_count(dir1.entries));
    EXPECT_EQ(3, alist_count(dir2.entries));

    cchfile_destruct(&file3);
    cchdir_destruct(&dir1);
    cchdir_destruct(&dir2);
    cchdir_destruct(&root);
    //fat_destruct(&fat);
}

TEST_F(ClusterChainDirectoryTest, CopyFile)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    struct cchdir root;
    cchdir_readroot(this->device, &fat, &root);

    DirectoryEntry orige;
    DirectoryEntry copye;

    DirectoryEntry de1;
    DirectoryEntry de2;

    struct cchdir dir1;
    struct cchdir dir2;

    struct cchfile orig;
    struct cchfile copy;

    EXPECT_TRUE(cchdir_adddir(&root, "dir1", &de1, &dir1));
    cchdir_write(&dir1, this->device);

    EXPECT_TRUE(cchdir_adddir(&root, "dir2", &de2, &dir2));
    cchdir_write(&dir2, this->device);

    EXPECT_TRUE(cchdir_addfile(&dir1, "dump.bin", &orige));

    EXPECT_EQ(2, alist_count(root.entries));
    EXPECT_EQ(3, alist_count(dir1.entries)); // including "." and ".." directories
    EXPECT_EQ(2, alist_count(dir2.entries));

    uint32_t i, nread;
    uint32_t len = 4096 * 4;
    uint8_t buf[len];
    for (i = 0; i < len; ++i) {
        buf[i] = i % 256;
    }    

    // Write to source file
    cchdir_getfile(&dir1, &orige, &orig);
    cchfile_write(this->device, &orig, 0, len, buf);

    EXPECT_TRUE(cchdir_copyfile(this->device, &dir1, &orige, &dir2));

    EXPECT_TRUE(cchdir_findentry(&dir1, "dump.bin", &orige));
    EXPECT_TRUE(cchdir_findentry(&dir2, "dump.bin", &copye));

    // Read from copy
    cchdir_getfile(&dir2, &copye, &copy);
    EXPECT_EQ(len, cchfile_getlen(&copy));
    cchfile_read(this->device, &copy, 0, len, &nread, buf);

    for (i = 0; i < len; ++i) {
        EXPECT_EQ(i % 256, buf[i]);
    }

    EXPECT_EQ(2, alist_count(root.entries));
    EXPECT_EQ(3, alist_count(dir1.entries));
    EXPECT_EQ(3, alist_count(dir2.entries));

    cchfile_destruct(&orig);
    cchfile_destruct(&copy);
    cchdir_destruct(&dir1);
    cchdir_destruct(&dir2);
    cchdir_destruct(&root);
    //fat_destruct(&fat);
}

TEST_F(ClusterChainDirectoryTest, CopyDirectory)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    struct cchdir root;
    cchdir_readroot(this->device, &fat, &root);

    DirectoryEntry fe;

    DirectoryEntry de1;
    DirectoryEntry de2;

    struct cchdir dir1;
    struct cchdir dir2;

    EXPECT_TRUE(cchdir_adddir(&root, "dir1", &de1, &dir1));
    cchdir_write(&dir1, this->device);

    EXPECT_TRUE(cchdir_adddir(&root, "dir2", &de2, &dir2));
    cchdir_write(&dir2, this->device);

    EXPECT_TRUE(cchdir_addfile(&dir1, "dump.bin", &fe));

    // Keep in mind that all directories except root include "." and ".." sub-directories
    EXPECT_EQ(2, alist_count(root.entries));
    EXPECT_EQ(3, alist_count(dir1.entries));
    EXPECT_EQ(2, alist_count(dir2.entries));

    uint32_t i, nread;
    uint32_t len = 4096 * 4 + 100;
    uint8_t buf[len];
    for (i = 0; i < len; ++i) {
        buf[i] = i % 256;
    }

    // Write to file
    struct cchfile file;
    cchdir_getfile(&dir1, &fe, &file);
    cchfile_write(this->device, &file, 0, len, buf);

    cchdir_write(&dir1, this->device);
    cchdir_write(&dir2, this->device);

    EXPECT_TRUE(cchdir_copydir(this->device, &root, &de1, &dir2));

    DirectoryEntry copyfe;
    DirectoryEntry copyde1;
    struct cchdir copydir1;
    struct cchfile copyfile;

    EXPECT_TRUE(cchdir_findentry(&root, "dir1", &de1));
    EXPECT_TRUE(cchdir_findentry(&root, "dir2", &de2));
    EXPECT_TRUE(cchdir_findentry(&dir1, "dump.bin", &fe));
    EXPECT_TRUE(cchdir_findentry(&dir2, "dir1", &copyde1));
    EXPECT_TRUE(cchdir_getdir(this->device, &fat, &copyde1, &copydir1));
    EXPECT_TRUE(cchdir_findentry(&copydir1, "dump.bin", &copyfe));

    EXPECT_EQ(2, alist_count(root.entries));
    EXPECT_EQ(3, alist_count(dir1.entries));
    EXPECT_EQ(3, alist_count(dir2.entries));
    EXPECT_EQ(3, alist_count(copydir1.entries));

    // Read from copy
    cchdir_getfile(&copydir1, &copyfe, &copyfile);
    EXPECT_EQ(len, cchfile_getlen(&copyfile));
    cchfile_read(this->device, &copyfile, 0, len, &nread, buf);

    for (i = 0; i < len; ++i) {
        EXPECT_EQ(i % 256, buf[i]);
    }

    // Writing to the copy doesn't affect to the origin file.
    buf[0] = 50;
    cchfile_write(this->device, &copyfile, 0, 1, buf);
    cchfile_read(this->device, &file, 0, 1, &nread, buf);
    EXPECT_EQ(0, buf[0]);
    cchfile_read(this->device, &copyfile, 0, 1, &nread, buf);
    EXPECT_EQ(50, buf[0]);

    cchfile_destruct(&copyfile);
    cchfile_destruct(&file);

    cchdir_destruct(&copydir1);
    cchdir_destruct(&dir1);
    cchdir_destruct(&dir2);
    cchdir_destruct(&root);

    //fat_destruct(&fat);
}
