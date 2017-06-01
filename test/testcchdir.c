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

    fdisk_create(G_DISK_FNAME, &disk);
    cchdir_formatdev(&disk, 1024 * 1024, 512, 1);

    fdisk_close(&disk);
}

MU_TEST_TEARDOWN(teardown)
{
    remove(G_DISK_FNAME);
    vfat_errno = 0;
}

MU_TEST(test_add_entry)
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

    MU_ASSERT_U32_EQ(0, alist_count(root.entries));

    struct lfnde e;
    lfnde_create(&e);
    cchdir_addentry(&root, &e);
    MU_ASSERT_U32_EQ(1, alist_count(root.entries));

    MU_ASSERT(br.rootdir_first_cluster != 0);
    MU_ASSERT_U32_EQ(root.capacity, cch_getsize(root.chain) / FAT_DIR_ENTRY_SIZE);

    cchdir_destruct(&root);
    fdisk_close(&disk);
}

MU_TEST(test_add_remove_entries)
{
    MU_PRINT_TEST_INFO();

    u32 i;
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cchdir root;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);
    cchdir_readroot(&disk, &fat, &root);

    MU_ASSERT_U32_EQ(0, alist_count(root.entries));

    struct lfnde e;
    for(i = 0; i < 100; ++i) {
        lfnde_create(&e);
        cchdir_addentry(&root, &e);
    }

    for(i = 0; i < 100; ++i) {
        MU_ASSERT_U32_EQ(100 - i, alist_count(root.entries));
        cchdir_getentry(&root, 0, &e);
        lfnde_destruct(&e);
        cchdir_removeentry(&root, 0);
    }

    cchdir_destruct(&root);
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
    u32 count = 0;

    do {
        free_before_add = fat_get_free_cluster_count(&fat);
        sprintf(namebuf, "this is test directory with index %d", count++);
        if (!cchdir_adddir(&root, namebuf, &e)) {
            MU_ASSERT_INT_EQ(EFATFULL, vfat_errno);
            MU_ASSERT_U32_EQ(free_before_add, fat_get_free_cluster_count(&fat));
            break;
        }
    } while (true);

    cchdir_destruct(&root);
    fdisk_close(&disk);
}

MU_TEST(test_remove_dir)
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

    u32 free_before = fat_get_free_cluster_count(&fat);
    u32 entries_before = alist_count(root.entries);

    struct lfnde e;
    const char *dir_name = "TestDirectory";
    cchdir_adddir(&root, dir_name, &e);
    MU_ASSERT_U32_EQ(free_before - 1, fat_get_free_cluster_count(&fat));
    MU_ASSERT_U32_EQ(entries_before + 1, alist_count(root.entries));
    MU_ASSERT(cchdir_findentry(&root, dir_name, &e) == true);

    MU_ASSERT(cchdir_removedir(&root, dir_name) == true);
    MU_ASSERT_U32_EQ(free_before, fat_get_free_cluster_count(&fat));
    MU_ASSERT_U32_EQ(entries_before, alist_count(root.entries));
    MU_ASSERT(cchdir_findentry(&root, dir_name, &e) == false);

    cchdir_destruct(&root);
    fdisk_close(&disk);
}

MU_TEST(test_unique_dir_name)
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

    MU_ASSERT(cchdir_adddir(&root, "TestDir#0000", &e) == true);
    MU_ASSERT(cchdir_adddir(&root, "TestDir#0000", &e) == false);
    MU_ASSERT_INT_EQ(EALREADYEXISTS, vfat_errno);

    cchdir_destruct(&root);
    fdisk_close(&disk);
}

MU_TEST(test_rename_file)
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

    cchdir_addfile(&root, "OldFileName", &e);
    cchdir_setname(&root, &e, "NewFileName");

    MU_ASSERT(cchdir_findentry(&root, "OldFileName", &e) == false);
    MU_ASSERT(cchdir_findentry(&root, "NewFileName", &e) == true);

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
    MU_RUN_TEST(test_remove_dir);
    MU_RUN_TEST(test_unique_dir_name);
    MU_RUN_TEST(test_rename_file);

    MU_REPORT();
}
