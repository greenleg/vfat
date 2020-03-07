#include "gtest/gtest.h"
#include "../include/common.h"
#include "../include/alist.h"
#include "../include/cch.h"
#include "../include/cchdir.h"
#include "../include/lfnde.h"

using namespace org::vfat;

class ClusterChainDirectoryTest : public ::testing::Test
{
protected:
    FileDisk *device;

    void SetUp() override
    {
        this->device = new FileDisk("disk0");

        this->device->Create();
        cchdir_formatdev(this->device, 1024 * 1024, 512, 1);
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
    Fat fat(&bootSector);
    struct cchdir root;

    bootSector.Read(this->device);
    fat.Read(this->device);
    cchdir_readroot(this->device, &fat, &root);

    EXPECT_EQ(0, alist_count(root.entries));

    struct lfnde e;
    lfnde_create(&e);
    cchdir_addentry(&root, &e);
    EXPECT_EQ(1, alist_count(root.entries));

    EXPECT_NE(0, bootSector.GetRootDirFirstCluster());
    EXPECT_EQ(root.capacity, cch_getsize(root.chain) / FAT_DIR_ENTRY_SIZE);

    cchdir_destruct(&root);
    //fat_destruct(&fat);
}

TEST_F(ClusterChainDirectoryTest, AddRemoveEntries)
{
    uint32_t i;
    BootSector bootSector;
    Fat fat(&bootSector);
    struct cchdir root;

    bootSector.Read(this->device);
    fat.Read(this->device);
    cchdir_readroot(this->device, &fat, &root);

    EXPECT_EQ(0, alist_count(root.entries));

    struct lfnde e;
    for(i = 0; i < 100; ++i) {
        lfnde_create(&e);
        cchdir_addentry(&root, &e);
    }

    for(i = 0; i < 100; ++i) {
        EXPECT_EQ(100 - i, alist_count(root.entries));
        cchdir_getentry(&root, 0, &e);
        lfnde_destruct(&e);
        cchdir_removeentry(&root, 0);
    }

    cchdir_destruct(&root);
    //fat_destruct(&fat);
}

TEST_F(ClusterChainDirectoryTest, AddSubDirectory)
{
    BootSector bootSector;
    Fat fat(&bootSector);
    struct cchdir root;

    bootSector.Read(this->device);
    fat.Read(this->device);
    cchdir_readroot(this->device, &fat, &root);

    const char *name = "A nice directory";
    struct lfnde e1;
    struct lfnde e2;
    struct cchdir dir;

    cchdir_adddir(&root, name, &e1, &dir);
    cchdir_findentry(&root, name, &e2);

    char namebuf[32];
    lfnde_getname(&e1, namebuf);
    EXPECT_STREQ(name, namebuf);
    lfnde_getname(&e2, namebuf);
    EXPECT_STREQ(name, namebuf);

    cchdir_destruct(&dir);
    cchdir_destruct(&root);
    //fat_destruct(&fat);
}

TEST_F(ClusterChainDirectoryTest, AddTooManyDirectories)
{
    BootSector bootSector;
    Fat fat(&bootSector);
    struct cchdir root;

    bootSector.Read(this->device);
    fat.Read(this->device);
    cchdir_readroot(this->device, &fat, &root);

    char namebuf[255];
    uint32_t free_before_add;
    struct lfnde e;
    struct cchdir dir;
    uint32_t count = 0;

    do {
        free_before_add = fat.GetFreeClusterCount();
        sprintf(namebuf, "this is test directory with index %d", count++);
        //bool errorWasThrown = false;
        try {
            cchdir_adddir(&root, namebuf, &e, &dir);
//            if (!cchdir_adddir(&root, namebuf, &e, &dir)) {
//                //EXPECT_EQ(EFATFULL, ::__vfat_errno);
//                ASSERT_EQ(free_before_add, fat.GetFreeClusterCount());
//                break;
//            }
        } catch (std::runtime_error error) {
            ASSERT_EQ(free_before_add, fat.GetFreeClusterCount());
            break;
        }
    } while (true);

    cchdir_destruct(&dir);
    cchdir_destruct(&root);
    //fat_destruct(&fat);
}

TEST_F(ClusterChainDirectoryTest, RemoveDirectory)
{
    BootSector bootSector;
    Fat fat(&bootSector);
    struct cchdir root;

    bootSector.Read(this->device);
    fat.Read(this->device);
    cchdir_readroot(this->device, &fat, &root);

    uint32_t free_before = fat.GetFreeClusterCount();
    uint32_t entries_before = alist_count(root.entries);

    struct lfnde e;
    struct cchdir dir;
    const char *dir_name = "testdir";
    cchdir_adddir(&root, dir_name, &e, &dir);
    EXPECT_EQ(free_before - 1, fat.GetFreeClusterCount());
    EXPECT_EQ(entries_before + 1, alist_count(root.entries));
    EXPECT_TRUE(cchdir_findentry(&root, dir_name, &e));

    EXPECT_TRUE(cchdir_removedir(&root, dir_name));
    EXPECT_EQ(free_before, fat.GetFreeClusterCount());
    EXPECT_EQ(entries_before, alist_count(root.entries));
    EXPECT_FALSE(cchdir_findentry(&root, dir_name, &e));

    cchdir_destruct(&root);
    //fat_destruct(&fat);
}

TEST_F(ClusterChainDirectoryTest, UniqueDirectoryName)
{
    BootSector bootSector;
    Fat fat(&bootSector);
    struct cchdir root;

    bootSector.Read(this->device);
    fat.Read(this->device);
    cchdir_readroot(this->device, &fat, &root);

    struct lfnde e;
    struct cchdir dir;

    EXPECT_TRUE(cchdir_adddir(&root, "home", &e, &dir));
    EXPECT_FALSE(cchdir_adddir(&root, "home", &e, &dir));
    EXPECT_EQ(EALREADYEXISTS, ::__vfat_errno);

    cchdir_destruct(&dir);

    // Reuse name
    EXPECT_TRUE(cchdir_removedir(&root, "home"));
    EXPECT_TRUE(cchdir_adddir(&root, "home", &e, &dir));

    cchdir_destruct(&dir);
    cchdir_destruct(&root);
    //fat_destruct(&fat);
}

TEST_F(ClusterChainDirectoryTest, RenameFile)
{
    BootSector bootSector;
    Fat fat(&bootSector);
    struct cchdir root;

    bootSector.Read(this->device);
    fat.Read(this->device);
    cchdir_readroot(this->device, &fat, &root);

    struct lfnde e;    

    cchdir_addfile(&root, "oldfile", &e);

    EXPECT_TRUE(cchdir_findentry(&root, "oldfile", &e));
    EXPECT_FALSE(cchdir_findentry(&root, "newfile", &e));

    cchdir_setname(this->device, &root, &e, "newfile");

    EXPECT_FALSE(cchdir_findentry(&root, "oldfile", &e));
    EXPECT_TRUE(cchdir_findentry(&root, "newfile", &e));

    cchdir_destruct(&root);
    //fat_destruct(&fat);
}

TEST_F(ClusterChainDirectoryTest, MoveFile)
{
    BootSector bootSector;
    Fat fat(&bootSector);
    struct cchdir root;

    bootSector.Read(this->device);
    fat.Read(this->device);
    cchdir_readroot(this->device, &fat, &root);

    struct lfnde fe;
    struct lfnde de;
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
    //fat_destruct(&fat);
}

TEST_F(ClusterChainDirectoryTest, MoveDirectory)
{
    BootSector bootSector;
    Fat fat(&bootSector);
    struct cchdir root;

    bootSector.Read(this->device);
    fat.Read(this->device);
    cchdir_readroot(this->device, &fat, &root);

    struct lfnde fe1;
    struct lfnde fe2;
    struct lfnde fe3;

    struct lfnde de1;
    struct lfnde de2;

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
    Fat fat(&bootSector);
    struct cchdir root;

    bootSector.Read(this->device);
    fat.Read(this->device);
    cchdir_readroot(this->device, &fat, &root);

    struct lfnde orige;
    struct lfnde copye;

    struct lfnde de1;
    struct lfnde de2;

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
    Fat fat(&bootSector);
    struct cchdir root;

    bootSector.Read(this->device);
    fat.Read(this->device);
    cchdir_readroot(this->device, &fat, &root);

    struct lfnde fe;

    struct lfnde de1;
    struct lfnde de2;

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

    struct lfnde copyfe;
    struct lfnde copyde1;
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
