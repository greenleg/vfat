#include <errno.h>

#include "common.h"
#include "minunit.h"
#include "alist.h"
#include "cch.h"
#include "cchdir.h"
#include "cchfile.h"
#include "lfnde.h"

static const char *G_DISK_FNAME = "/home/pavel/projects/vfat/test/disk0";

MU_TEST_SETUP(setup)
{
    struct fdisk disk;

    fdisk_create(G_DISK_FNAME, &disk);
    cchdir_formatdev(&disk, 1024 * 1024, 512, 1);

    fdisk_close(&disk);
}

MU_TEST_TEARDOWN(teardown)
{
    remove(G_DISK_FNAME);
    vfat_errno = 0;
}

MU_TEST(test_set_length)
{
    MU_PRINT_TEST_INFO();

    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);
    cchdir_readroot(&disk, &fat, &root);

    struct lfnde e;
    struct cchfile file;

    cchdir_addfile(&root, "index.htm", &e);
    cchdir_getfile(&disk, &root, &e, &file);

    MU_ASSERT_U32_EQ(0, cchfile_getlen(&file));
    cchfile_setlen(&file, 100);
    MU_ASSERT_U32_EQ(100, cchfile_getlen(&file));
    MU_ASSERT(cch_getsize(file.chain) >= 100);

    cchfile_destruct(&file);
    cchdir_destruct(&root);
    fdisk_close(&disk);
}

MU_TEST(test_read_write)
{
    MU_PRINT_TEST_INFO();

    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);
    cchdir_readroot(&disk, &fat, &root);

    struct lfnde e;
    struct cchfile file;

    cchdir_addfile(&root, "dump.bin", &e);
    cchdir_getfile(&disk, &root, &e, &file);

    u32 i;
    u32 len = 10000;
    u8 writebuf[len];
    u8 readbuf[len];

    for (i = 0; i < len; ++i) {
        writebuf[i] = i % 256;
    }

    // Write to device
    cchfile_write(&disk, &file, 0, len, writebuf);

    // Read from device
    cchfile_read(&disk, &file, 0, len, readbuf);

    for (i = 0; i < len; ++i) {
        MU_ASSERT_U32_EQ(i % 256, readbuf[i]);
    }

    // Read too long
    MU_ASSERT(cchfile_read(&disk, &file, 0, len + 1, readbuf) == false);
    MU_ASSERT_INT_EQ(EIO, vfat_errno);

    cchfile_destruct(&file);
    cchdir_destruct(&root);
    fdisk_close(&disk);
}    

MU_TEST_SUITE(cchfile_test_suite)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);
    MU_RESET();

    MU_RUN_TEST(test_set_length);
    MU_RUN_TEST(test_read_write);

    MU_REPORT();
}
