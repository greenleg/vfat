#include "minunit.h"
#include "../include/common.h"
#include "../include/alist.h"
#include "../include/cch.h"
#include "../include/cchdir.h"
#include "../include/lfnde.h"

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
    ::__vfat_errno = 0;
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
    struct lfnde e1;
    struct lfnde e2;
    struct cchdir dir;

    cchdir_adddir(&root, name, &e1, &dir);
    cchdir_findentry(&root, name, &e2);

    char namebuf[32];
    lfnde_getname(&e1, namebuf);
    MU_ASSERT_STRING_EQ(name, namebuf);
    lfnde_getname(&e2, namebuf);
    MU_ASSERT_STRING_EQ(name, namebuf);

    cchdir_destruct(&dir);
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
    struct cchdir dir;
    u32 count = 0;

    do {
        free_before_add = fat_get_free_cluster_count(&fat);
        sprintf(namebuf, "this is test directory with index %d", count++);
        if (!cchdir_adddir(&root, namebuf, &e, &dir)) {
            MU_ASSERT_INT_EQ(EFATFULL, ::__vfat_errno);
            MU_ASSERT_U32_EQ(free_before_add, fat_get_free_cluster_count(&fat));
            break;
        }
    } while (true);

    cchdir_destruct(&dir);
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
    struct cchdir dir;
    const char *dir_name = "testdir";
    cchdir_adddir(&root, dir_name, &e, &dir);
    MU_ASSERT_U32_EQ(free_before - 1, fat_get_free_cluster_count(&fat));
    MU_ASSERT_U32_EQ(entries_before + 1, alist_count(root.entries));
    MU_ASSERT(cchdir_findentry(&root, dir_name, &e));

    MU_ASSERT(cchdir_removedir(&root, dir_name));
    MU_ASSERT_U32_EQ(free_before, fat_get_free_cluster_count(&fat));
    MU_ASSERT_U32_EQ(entries_before, alist_count(root.entries));
    MU_ASSERT(!cchdir_findentry(&root, dir_name, &e));

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
    struct cchdir dir;

    MU_ASSERT(cchdir_adddir(&root, "home", &e, &dir));
    MU_ASSERT(!cchdir_adddir(&root, "home", &e, &dir));
    MU_ASSERT_INT_EQ(EALREADYEXISTS, ::__vfat_errno);

    cchdir_destruct(&dir);

    // Reuse name
    MU_ASSERT(cchdir_removedir(&root, "home"));
    MU_ASSERT(cchdir_adddir(&root, "home", &e, &dir));

    cchdir_destruct(&dir);
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

    cchdir_addfile(&root, "oldfile", &e);

    MU_ASSERT(cchdir_findentry(&root, "oldfile", &e) == true);
    MU_ASSERT(cchdir_findentry(&root, "newfile", &e) == false);

    cchdir_setname(&disk, &root, &e, "newfile");

    MU_ASSERT(cchdir_findentry(&root, "oldfile", &e) == false);
    MU_ASSERT(cchdir_findentry(&root, "newfile", &e) == true);

    cchdir_destruct(&root);
    fdisk_close(&disk);
}

MU_TEST(test_move_file)
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

    struct lfnde fe;
    struct lfnde de;
    struct cchdir dir;    

    MU_ASSERT(cchdir_adddir(&root, "home", &de, &dir));
    MU_ASSERT(cchdir_addfile(&root, "dump.bin", &fe));

    MU_ASSERT(cchdir_move(&disk, &root, &fe, &dir, "dump2.bin"));

    MU_ASSERT_U32_EQ(1, alist_count(root.entries));
    MU_ASSERT_U32_EQ(3, alist_count(dir.entries));  // including "." and ".." directories.
    MU_ASSERT(cchdir_findentry(&root, "home", &de));
    MU_ASSERT(!cchdir_findentry(&root, "dump2.bin", &de));
    MU_ASSERT(cchdir_findentry(&dir, "dump2.bin", &de));

    cchdir_destruct(&dir);
    cchdir_destruct(&root);
    fdisk_close(&disk);
}

MU_TEST(test_move_dir)
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

    struct lfnde fe1;
    struct lfnde fe2;
    struct lfnde fe3;

    struct lfnde de1;
    struct lfnde de2;

    struct cchdir dir1;
    struct cchdir dir2;

    struct cchfile file3;

    MU_ASSERT(cchdir_adddir(&root, "dir1", &de1, &dir1));
    cchdir_write(&dir1, &disk);

    MU_ASSERT(cchdir_adddir(&root, "dir2", &de2, &dir2));
    cchdir_write(&dir2, &disk);

    MU_ASSERT(cchdir_addfile(&dir1, "dump1.bin", &fe1));
    MU_ASSERT(cchdir_addfile(&dir1, "dump2.bin", &fe2));
    MU_ASSERT(cchdir_addfile(&dir1, "dump3.bin", &fe3));

    cchdir_getfile(&dir1, &fe3, &file3);

    MU_ASSERT_U32_EQ(2, alist_count(root.entries));
    MU_ASSERT_U32_EQ(5, alist_count(dir1.entries)); // including "." and ".." directories
    MU_ASSERT_U32_EQ(2, alist_count(dir2.entries));

    MU_ASSERT(cchdir_move(&disk, &root, &de1, &dir2, "dir1"));

    cchdir_write(&dir1, &disk);
    cchdir_write(&dir2, &disk);

    cchdir_destruct(&dir1);
    cchdir_destruct(&dir2);

    MU_ASSERT(cchdir_findentry(&root, "dir2", &de2));
    cchdir_getdir(&disk, &fat, &de2, &dir2);

    MU_ASSERT(cchdir_findentry(&dir2, "dir1", &de1));
    cchdir_getdir(&disk, &fat, &de1, &dir1);

    MU_ASSERT_U32_EQ(1, alist_count(root.entries));
    MU_ASSERT_U32_EQ(5, alist_count(dir1.entries));
    MU_ASSERT_U32_EQ(3, alist_count(dir2.entries));

    cchfile_destruct(&file3);
    cchdir_destruct(&dir1);
    cchdir_destruct(&dir2);
    cchdir_destruct(&root);
    fdisk_close(&disk);
}

MU_TEST(test_copy_file)
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

    struct lfnde orige;
    struct lfnde copye;

    struct lfnde de1;
    struct lfnde de2;

    struct cchdir dir1;
    struct cchdir dir2;

    struct cchfile orig;
    struct cchfile copy;

    MU_ASSERT(cchdir_adddir(&root, "dir1", &de1, &dir1));
    cchdir_write(&dir1, &disk);

    MU_ASSERT(cchdir_adddir(&root, "dir2", &de2, &dir2));
    cchdir_write(&dir2, &disk);

    MU_ASSERT(cchdir_addfile(&dir1, "dump.bin", &orige));

    MU_ASSERT_U32_EQ(2, alist_count(root.entries));
    MU_ASSERT_U32_EQ(3, alist_count(dir1.entries)); // including "." and ".." directories
    MU_ASSERT_U32_EQ(2, alist_count(dir2.entries));

    u32 i, nread;
    u32 len = 4096 * 4;
    u8 buf[len];
    for (i = 0; i < len; ++i) {
        buf[i] = i % 256;
    }    

    // Write to source file
    cchdir_getfile(&dir1, &orige, &orig);
    cchfile_write(&disk, &orig, 0, len, buf);

    MU_ASSERT(cchdir_copyfile(&disk, &dir1, &orige, &dir2));

    MU_ASSERT(cchdir_findentry(&dir1, "dump.bin", &orige));
    MU_ASSERT(cchdir_findentry(&dir2, "dump.bin", &copye));

    // Read from copy
    cchdir_getfile(&dir2, &copye, &copy);
    MU_ASSERT_U32_EQ(len, cchfile_getlen(&copy));
    cchfile_read(&disk, &copy, 0, len, &nread, buf);

    for (i = 0; i < len; ++i) {
        MU_ASSERT_U32_EQ(i % 256, buf[i]);
    }

    MU_ASSERT_U32_EQ(2, alist_count(root.entries));
    MU_ASSERT_U32_EQ(3, alist_count(dir1.entries));
    MU_ASSERT_U32_EQ(3, alist_count(dir2.entries));

    cchfile_destruct(&orig);
    cchfile_destruct(&copy);
    cchdir_destruct(&dir1);
    cchdir_destruct(&dir2);
    cchdir_destruct(&root);
    fdisk_close(&disk);
}

MU_TEST(test_copy_dir)
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

    struct lfnde fe;

    struct lfnde de1;
    struct lfnde de2;

    struct cchdir dir1;
    struct cchdir dir2;

    MU_ASSERT(cchdir_adddir(&root, "dir1", &de1, &dir1));
    cchdir_write(&dir1, &disk);

    MU_ASSERT(cchdir_adddir(&root, "dir2", &de2, &dir2));
    cchdir_write(&dir2, &disk);

    MU_ASSERT(cchdir_addfile(&dir1, "dump.bin", &fe));

    // Keep in mind that all directories except root include "." and ".." sub-directories
    MU_ASSERT_U32_EQ(2, alist_count(root.entries));
    MU_ASSERT_U32_EQ(3, alist_count(dir1.entries));
    MU_ASSERT_U32_EQ(2, alist_count(dir2.entries));

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

    MU_ASSERT(cchdir_copydir(&disk, &root, &de1, &dir2));

    struct lfnde copyfe;
    struct lfnde copyde1;
    struct cchdir copydir1;
    struct cchfile copyfile;

    MU_ASSERT(cchdir_findentry(&root, "dir1", &de1));
    MU_ASSERT(cchdir_findentry(&root, "dir2", &de2));
    MU_ASSERT(cchdir_findentry(&dir1, "dump.bin", &fe));
    MU_ASSERT(cchdir_findentry(&dir2, "dir1", &copyde1));
    MU_ASSERT(cchdir_getdir(&disk, &fat, &copyde1, &copydir1));
    MU_ASSERT(cchdir_findentry(&copydir1, "dump.bin", &copyfe));

    MU_ASSERT_U32_EQ(2, alist_count(root.entries));
    MU_ASSERT_U32_EQ(3, alist_count(dir1.entries));
    MU_ASSERT_U32_EQ(3, alist_count(dir2.entries));
    MU_ASSERT_U32_EQ(3, alist_count(copydir1.entries));

    // Read from copy
    cchdir_getfile(&copydir1, &copyfe, &copyfile);
    MU_ASSERT_U32_EQ(len, cchfile_getlen(&copyfile));
    cchfile_read(&disk, &copyfile, 0, len, &nread, buf);

    for (i = 0; i < len; ++i) {
        MU_ASSERT_U32_EQ(i % 256, buf[i]);
    }

    // Writing to the copy doesn't affect to the origin file.
    buf[0] = 50;
    cchfile_write(&disk, &copyfile, 0, 1, buf);
    cchfile_read(&disk, &file, 0, 1, &nread, buf);
    MU_ASSERT_U32_EQ(0, buf[0]);
    cchfile_read(&disk, &copyfile, 0, 1, &nread, buf);
    MU_ASSERT_U32_EQ(50, buf[0]);

    cchfile_destruct(&copyfile);
    cchfile_destruct(&file);

    cchdir_destruct(&copydir1);
    cchdir_destruct(&dir1);
    cchdir_destruct(&dir2);
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
    MU_RUN_TEST(test_move_file);
    MU_RUN_TEST(test_move_dir);
    MU_RUN_TEST(test_copy_file);
    MU_RUN_TEST(test_copy_dir);

    MU_REPORT();
}
