#include "common.h"
#include "minunit.h"
#include "alist.h"
#include "cch.h"
#include "cchdir.h"
#include "lfnde.h"

static const char *G_DISK_FNAME = "/home/pavel/projects/vfat/test/disk0";

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
    cchdir_createroot(&fat, &dir);

    cchdir_write(&dir, &disk);
    fat_write(&fat, &disk);

    cchdir_destruct(&dir);
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
    lfnde_create(&e);
    cchdir_addentry(&dir, &e);
    MU_ASSERT_U32_EQ(1, alist_count(dir.entries));

    MU_ASSERT(br.rootdir_first_cluster != 0);
    MU_ASSERT_U32_EQ(dir.capacity, cch_getsize(dir.chain) / FAT_DIR_ENTRY_SIZE);

    cchdir_destruct(&dir);
    fdisk_close(&disk);
}

MU_TEST(test_add_remove_entries)
{
    MU_PRINT_TEST_INFO();

    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir dir;
    u32 i;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    cchdir_readroot(&disk, &fat, &dir);

    MU_ASSERT_U32_EQ(0, alist_count(dir.entries));

    struct lfnde e;
    for(i = 0; i < 100; ++i) {
        lfnde_create(&e);
        cchdir_addentry(&dir, &e);
    }

    for(i = 0; i < 100; ++i) {
        MU_ASSERT_U32_EQ(100 - i, alist_count(dir.entries));
        cchdir_getentry(&dir, 0, &e);
        lfnde_destruct(&e);
        cchdir_removeentry(&dir, 0);
    }

    cchdir_destruct(&dir);
    fdisk_close(&disk);
}

MU_TEST(test_add_subdir)
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

    const char *name = "A nice directory";
    struct lfnde e;
    struct lfnde d;

    cchdir_adddir(&root, name, &e);
    cchdir_findentry(&root, name, &d);

    char namebuf[32];
    lfnde_getname(&e, namebuf);
    MU_ASSERT_STRING_EQ(name, namebuf);
    lfnde_getname(&d, namebuf);
    MU_ASSERT_STRING_EQ(name, namebuf);

    cchdir_destruct(&root);
    fdisk_close(&disk);
}

MU_TEST(test_add_too_many_directories)
{
    MU_PRINT_TEST_INFO();

    u32 count = 0;
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);
    cchdir_readroot(&disk, &fat, &root);

    char namebuf[255];
    u32 free_before_add;
    struct lfnde e;
    int res;

    do {
        free_before_add = fat_get_free_cluster_count(&fat);

        sprintf(namebuf, "this is test directory with index %d", count++);
        res = cchdir_adddir(&root, namebuf, &e);

        if (res == -1) {
            MU_ASSERT_U32_EQ(free_before_add, fat_get_free_cluster_count(&fat));
            break;
        }
    } while (true);

    cchdir_destruct(&root);
    fdisk_close(&disk);
}

MU_TEST_SUITE(cchdir_test_suite)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);
    MU_RESET();

    MU_RUN_TEST(test_add_entry);
    MU_RUN_TEST(test_add_remove_entries);
    MU_RUN_TEST(test_add_subdir);
    MU_RUN_TEST(test_add_too_many_directories);

    MU_REPORT();
}
