#include "gtest/gtest.h"
#include "../include/common.h"
#include "../include/alist.h"
#include "../include/cch.h"
#include "../include/cchdir.h"
#include "../include/lfnde.h"

class ClusterChainDirectoryTest : public ::testing::Test
{
protected:
    const char *DISK_FNAME = "disk0";

    void SetUp() override
    {
        struct fdisk disk;

        fdisk_create(DISK_FNAME, &disk);
        cchdir_formatdev(&disk, 1024 * 1024, 512, 1);

        fdisk_close(&disk);
    }

    void TearDown() override
    {
        remove(DISK_FNAME);
        ::__vfat_errno = 0;
    }
};

TEST_F(ClusterChainDirectoryTest, AddEntry)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);
    cchdir_readroot(&disk, &fat, &root);

    EXPECT_EQ(0, alist_count(root.entries));

    struct lfnde e;
    lfnde_create(&e);
    cchdir_addentry(&root, &e);
    EXPECT_EQ(1, alist_count(root.entries));

    EXPECT_NE(0, br.rootdir_first_cluster);
    EXPECT_EQ(root.capacity, cch_getsize(root.chain) / FAT_DIR_ENTRY_SIZE);

    cchdir_destruct(&root);
    fdisk_close(&disk);
}

TEST_F(ClusterChainDirectoryTest, AddRemoveEntries)
{
    u32 i;
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);
    cchdir_readroot(&disk, &fat, &root);

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
    fdisk_close(&disk);
}

TEST_F(ClusterChainDirectoryTest, AddSubDirectory)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);
    cchdir_readroot(&disk, &fat, &root);

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
    fdisk_close(&disk);
}

TEST_F(ClusterChainDirectoryTest, AddTooManyDirectories)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);
    cchdir_readroot(&disk, &fat, &root);

    char namebuf[255];
    u32 free_before_add;
    struct lfnde e;
    struct cchdir dir;
    u32 count = 0;

    do {
        free_before_add = fat_get_free_cluster_count(&fat);
        sprintf(namebuf, "this is test directory with index %d", count++);
        if (!cchdir_adddir(&root, namebuf, &e, &dir)) {
            EXPECT_EQ(EFATFULL, ::__vfat_errno);
            EXPECT_EQ(free_before_add, fat_get_free_cluster_count(&fat));
            break;
        }
    } while (true);

    cchdir_destruct(&dir);
    cchdir_destruct(&root);
    fdisk_close(&disk);
}

TEST_F(ClusterChainDirectoryTest, RemoveDirectory)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);
    cchdir_readroot(&disk, &fat, &root);

    u32 free_before = fat_get_free_cluster_count(&fat);
    u32 entries_before = alist_count(root.entries);

    struct lfnde e;
    struct cchdir dir;
    const char *dir_name = "testdir";
    cchdir_adddir(&root, dir_name, &e, &dir);
    EXPECT_EQ(free_before - 1, fat_get_free_cluster_count(&fat));
    EXPECT_EQ(entries_before + 1, alist_count(root.entries));
    EXPECT_TRUE(cchdir_findentry(&root, dir_name, &e));

    EXPECT_TRUE(cchdir_removedir(&root, dir_name));
    EXPECT_EQ(free_before, fat_get_free_cluster_count(&fat));
    EXPECT_EQ(entries_before, alist_count(root.entries));
    EXPECT_FALSE(cchdir_findentry(&root, dir_name, &e));

    cchdir_destruct(&root);
    fdisk_close(&disk);
}

TEST_F(ClusterChainDirectoryTest, UniqueDirectoryName)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);
    cchdir_readroot(&disk, &fat, &root);

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
    fdisk_close(&disk);
}

TEST_F(ClusterChainDirectoryTest, RenameFile)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);
    cchdir_readroot(&disk, &fat, &root);

    struct lfnde e;    

    cchdir_addfile(&root, "oldfile", &e);

    EXPECT_TRUE(cchdir_findentry(&root, "oldfile", &e));
    EXPECT_FALSE(cchdir_findentry(&root, "newfile", &e));

    cchdir_setname(&disk, &root, &e, "newfile");

    EXPECT_FALSE(cchdir_findentry(&root, "oldfile", &e));
    EXPECT_TRUE(cchdir_findentry(&root, "newfile", &e));

    cchdir_destruct(&root);
    fdisk_close(&disk);
}

TEST_F(ClusterChainDirectoryTest, MoveFile)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);
    cchdir_readroot(&disk, &fat, &root);

    struct lfnde fe;
    struct lfnde de;
    struct cchdir dir;    

    EXPECT_TRUE(cchdir_adddir(&root, "home", &de, &dir));
    EXPECT_TRUE(cchdir_addfile(&root, "dump.bin", &fe));

    EXPECT_TRUE(cchdir_move(&disk, &root, &fe, &dir, "dump2.bin"));

    EXPECT_EQ(1, alist_count(root.entries));
    EXPECT_EQ(3, alist_count(dir.entries));  // including "." and ".." directories.
    EXPECT_TRUE(cchdir_findentry(&root, "home", &de));
    EXPECT_FALSE(cchdir_findentry(&root, "dump2.bin", &de));
    EXPECT_TRUE(cchdir_findentry(&dir, "dump2.bin", &de));

    cchdir_destruct(&dir);
    cchdir_destruct(&root);
    fdisk_close(&disk);
}

TEST_F(ClusterChainDirectoryTest, MoveDirectory)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);
    cchdir_readroot(&disk, &fat, &root);

    struct lfnde fe1;
    struct lfnde fe2;
    struct lfnde fe3;

    struct lfnde de1;
    struct lfnde de2;

    struct cchdir dir1;
    struct cchdir dir2;

    struct cchfile file3;

    EXPECT_TRUE(cchdir_adddir(&root, "dir1", &de1, &dir1));
    cchdir_write(&dir1, &disk);

    EXPECT_TRUE(cchdir_adddir(&root, "dir2", &de2, &dir2));
    cchdir_write(&dir2, &disk);

    EXPECT_TRUE(cchdir_addfile(&dir1, "dump1.bin", &fe1));
    EXPECT_TRUE(cchdir_addfile(&dir1, "dump2.bin", &fe2));
    EXPECT_TRUE(cchdir_addfile(&dir1, "dump3.bin", &fe3));

    cchdir_getfile(&dir1, &fe3, &file3);

    EXPECT_EQ(2, alist_count(root.entries));
    EXPECT_EQ(5, alist_count(dir1.entries)); // including "." and ".." directories
    EXPECT_EQ(2, alist_count(dir2.entries));

    EXPECT_TRUE(cchdir_move(&disk, &root, &de1, &dir2, "dir1"));

    cchdir_write(&dir1, &disk);
    cchdir_write(&dir2, &disk);

    cchdir_destruct(&dir1);
    cchdir_destruct(&dir2);

    EXPECT_TRUE(cchdir_findentry(&root, "dir2", &de2));
    cchdir_getdir(&disk, &fat, &de2, &dir2);

    EXPECT_TRUE(cchdir_findentry(&dir2, "dir1", &de1));
    cchdir_getdir(&disk, &fat, &de1, &dir1);

    EXPECT_EQ(1, alist_count(root.entries));
    EXPECT_EQ(5, alist_count(dir1.entries));
    EXPECT_EQ(3, alist_count(dir2.entries));

    cchfile_destruct(&file3);
    cchdir_destruct(&dir1);
    cchdir_destruct(&dir2);
    cchdir_destruct(&root);
    fdisk_close(&disk);
}

TEST_F(ClusterChainDirectoryTest, CopyFile)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);
    cchdir_readroot(&disk, &fat, &root);

    struct lfnde orige;
    struct lfnde copye;

    struct lfnde de1;
    struct lfnde de2;

    struct cchdir dir1;
    struct cchdir dir2;

    struct cchfile orig;
    struct cchfile copy;

    EXPECT_TRUE(cchdir_adddir(&root, "dir1", &de1, &dir1));
    cchdir_write(&dir1, &disk);

    EXPECT_TRUE(cchdir_adddir(&root, "dir2", &de2, &dir2));
    cchdir_write(&dir2, &disk);

    EXPECT_TRUE(cchdir_addfile(&dir1, "dump.bin", &orige));

    EXPECT_EQ(2, alist_count(root.entries));
    EXPECT_EQ(3, alist_count(dir1.entries)); // including "." and ".." directories
    EXPECT_EQ(2, alist_count(dir2.entries));

    u32 i, nread;
    u32 len = 4096 * 4;
    u8 buf[len];
    for (i = 0; i < len; ++i) {
        buf[i] = i % 256;
    }    

    // Write to source file
    cchdir_getfile(&dir1, &orige, &orig);
    cchfile_write(&disk, &orig, 0, len, buf);

    EXPECT_TRUE(cchdir_copyfile(&disk, &dir1, &orige, &dir2));

    EXPECT_TRUE(cchdir_findentry(&dir1, "dump.bin", &orige));
    EXPECT_TRUE(cchdir_findentry(&dir2, "dump.bin", &copye));

    // Read from copy
    cchdir_getfile(&dir2, &copye, &copy);
    EXPECT_EQ(len, cchfile_getlen(&copy));
    cchfile_read(&disk, &copy, 0, len, &nread, buf);

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
    fdisk_close(&disk);
}

TEST_F(ClusterChainDirectoryTest, CopyDirectory)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);
    cchdir_readroot(&disk, &fat, &root);

    struct lfnde fe;

    struct lfnde de1;
    struct lfnde de2;

    struct cchdir dir1;
    struct cchdir dir2;

    EXPECT_TRUE(cchdir_adddir(&root, "dir1", &de1, &dir1));
    cchdir_write(&dir1, &disk);

    EXPECT_TRUE(cchdir_adddir(&root, "dir2", &de2, &dir2));
    cchdir_write(&dir2, &disk);

    EXPECT_TRUE(cchdir_addfile(&dir1, "dump.bin", &fe));

    // Keep in mind that all directories except root include "." and ".." sub-directories
    EXPECT_EQ(2, alist_count(root.entries));
    EXPECT_EQ(3, alist_count(dir1.entries));
    EXPECT_EQ(2, alist_count(dir2.entries));

    u32 i, nread;
    u32 len = 4096 * 4 + 100;
    u8 buf[len];
    for (i = 0; i < len; ++i) {
        buf[i] = i % 256;
    }

    // Write to file
    struct cchfile file;
    cchdir_getfile(&dir1, &fe, &file);
    cchfile_write(&disk, &file, 0, len, buf);

    cchdir_write(&dir1, &disk);
    cchdir_write(&dir2, &disk);

    EXPECT_TRUE(cchdir_copydir(&disk, &root, &de1, &dir2));

    struct lfnde copyfe;
    struct lfnde copyde1;
    struct cchdir copydir1;
    struct cchfile copyfile;

    EXPECT_TRUE(cchdir_findentry(&root, "dir1", &de1));
    EXPECT_TRUE(cchdir_findentry(&root, "dir2", &de2));
    EXPECT_TRUE(cchdir_findentry(&dir1, "dump.bin", &fe));
    EXPECT_TRUE(cchdir_findentry(&dir2, "dir1", &copyde1));
    EXPECT_TRUE(cchdir_getdir(&disk, &fat, &copyde1, &copydir1));
    EXPECT_TRUE(cchdir_findentry(&copydir1, "dump.bin", &copyfe));

    EXPECT_EQ(2, alist_count(root.entries));
    EXPECT_EQ(3, alist_count(dir1.entries));
    EXPECT_EQ(3, alist_count(dir2.entries));
    EXPECT_EQ(3, alist_count(copydir1.entries));

    // Read from copy
    cchdir_getfile(&copydir1, &copyfe, &copyfile);
    EXPECT_EQ(len, cchfile_getlen(&copyfile));
    cchfile_read(&disk, &copyfile, 0, len, &nread, buf);

    for (i = 0; i < len; ++i) {
        EXPECT_EQ(i % 256, buf[i]);
    }

    // Writing to the copy doesn't affect to the origin file.
    buf[0] = 50;
    cchfile_write(&disk, &copyfile, 0, 1, buf);
    cchfile_read(&disk, &file, 0, 1, &nread, buf);
    EXPECT_EQ(0, buf[0]);
    cchfile_read(&disk, &copyfile, 0, 1, &nread, buf);
    EXPECT_EQ(50, buf[0]);

    cchfile_destruct(&copyfile);
    cchfile_destruct(&file);

    cchdir_destruct(&copydir1);
    cchdir_destruct(&dir1);
    cchdir_destruct(&dir2);
    cchdir_destruct(&root);

    fdisk_close(&disk);
}
