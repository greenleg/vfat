#include <errno.h>
#include "gtest/gtest.h"
#include "../include/common.h"
#include "../include/alist.h"
#include "../include/cch.h"
#include "../include/cchdir.h"
#include "../include/cchfile.h"
#include "../include/lfnde.h"

class ClusterChainFileTest : public ::testing::Test
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

TEST_F(ClusterChainFileTest, SetLength)
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
    struct cchfile file;

    cchdir_addfile(&root, "index.htm", &e);
    cchdir_getfile(&root, &e, &file);

    EXPECT_EQ(0, cchfile_getlen(&file));
    cchfile_setlen(&file, 100);
    EXPECT_EQ(100, cchfile_getlen(&file));
    EXPECT_GE(cch_getsize(file.chain), 100);

    cchfile_destruct(&file);
    cchdir_destruct(&root);
    fdisk_close(&disk);
}

TEST_F(ClusterChainFileTest, ReadWrite)
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
    struct cchfile file;

    cchdir_addfile(&root, "dump.bin", &e);
    cchdir_getfile(&root, &e, &file);

    u32 i, nread;
    u32 len = 10000;
    u8 writebuf[len];
    u8 readbuf[len];

    for (i = 0; i < len; ++i) {
        writebuf[i] = i % 256;
    }

    // Write to device
    cchfile_write(&disk, &file, 0, len, writebuf);

    // Read from device
    cchfile_read(&disk, &file, 0, len, &nread, readbuf);
    EXPECT_EQ(len, nread);

    for (i = 0; i < len; ++i) {
        EXPECT_EQ(i % 256, readbuf[i]);
    }

    // Read too long
    cchfile_read(&disk, &file, 0, len + 1, &nread, readbuf);
    EXPECT_EQ(len, nread);

    cchfile_destruct(&file);
    cchdir_destruct(&root);
    fdisk_close(&disk);
}
