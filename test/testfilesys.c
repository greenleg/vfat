#include "minunit.h"
#include "filesys.h"

static const char *G_DISK_FNAME = "/home/pavel/projects/vfat/test/disk0";

MU_TEST_SETUP(setup)
{
    struct fdisk dev;
    struct filesys fs;

    fdisk_create(G_DISK_FNAME, &dev);
    filesys_format(&dev, 1024 * 1024, 512, 1, &fs);

    filesys_destruct(&fs);
    fdisk_close(&dev);
}

MU_TEST_TEARDOWN(teardown)
{
    remove(G_DISK_FNAME);
}

MU_TEST(test_mkdir)
{
    MU_PRINT_TEST_INFO();

    struct fdisk dev;
    struct filesys fs;

    fdisk_open(G_DISK_FNAME, &dev);
    filesys_open(&dev, &fs);

    filesys_mkdir(&fs, "/home/pavel/projects/vfat");

    filesys_destruct(&fs);
    fdisk_close(&dev);
}

MU_TEST_SUITE(filesys_test_suite)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);
    MU_RESET();

    MU_RUN_TEST(test_mkdir);

    MU_REPORT();
}
