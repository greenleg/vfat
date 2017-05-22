#include "common.h"
#include "minunit.h"
#include "alist.h"
#include "cch.h"
#include "cchdir.h"
#include "lfnde.h"

static const char*  G_DISK_FNAME = "/home/pavel/projects/vfat/test/disk0";

MU_TEST_SETUP(setup)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir dir;

    fdisk_create(G_DISK_FNAME, &disk);

    vbr_create(&br, 1024 * 1024, 512, 1);
    vbr_write(&br, &disk);

    fat_create(&br, &fat);
    fat_write(&fat, &disk);

    cchdir_createroot(&fat, &dir);
    cchdir_write(&dir, &disk);

    cchdir_free(&dir);
    fdisk_close(&disk);
}

MU_TEST_TEARDOWN(teardown)
{
    remove(G_DISK_FNAME);
}

MU_TEST(test_add_entry)
{
    MU_PRINT_TEST_INFO();

    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir dir;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    cchdir_readroot(&disk, &fat, &dir);

    MU_ASSERT_U32_EQ(0, alist_count(dir.entries));

    struct lfnde e;
    //cchdir_create("some name", true);
    cchdir_add(&dir, &e);
    MU_ASSERT_U32_EQ(1, alist_count(dir.entries));

    cchdir_free(&dir);
    fdisk_close(&disk);
}

MU_TEST_SUITE(cchdir_test_suite)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);
    MU_RESET();

    MU_RUN_TEST(test_add_entry);

    MU_REPORT();
}
