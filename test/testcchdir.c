#include "common.h"
#include "minunit.h"
#include "clusterchain.h"
#include "cchdir.h"

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

    cchdir_createroot(&fat, &dir);
    cchdir_readroot(&disk, &dir);

    MU_ASSERT_U32_EQ(0, dir.entries->cnt);


    /*        FatDirectoryEntry* e = FatDirectoryEntry::create(false);
            dir->addEntry(e);
            assert(dir->getEntryCount() == 1);
            delete e;*/

    fdisk_close(&disk);
}

MU_TEST_SUITE(cch_test_suite)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);
    MU_RESET();

    MU_RUN_TEST(test_add_entry);

    MU_REPORT();
}
