#include "minunit.h"
#include "../include/filesys.h"

static const char *G_DISK_FNAME = "/home/pavel/projects/vfat/test/disk0";

static void printdir(struct filesys *fs, struct vdir *dir, int level)
{
    struct vdir *subdir;
    struct vdirent e;
    while(filesys_readdir(dir, &e)) {
        if (strcmp(".", e.name) == 0 || strcmp("..", e.name) == 0) {
            continue;
        }

        for (int i = 0; i < level; ++i) {
            printf("*\t");
        }

        printf("%s\n", e.name);

        subdir = filesys_getdir(fs, dir, e.name);
        printdir(fs, subdir, level + 1);
        filesys_closedir(fs, subdir);
    }
}

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

MU_TEST(test_readdir)
{
    MU_PRINT_TEST_INFO();

    struct fdisk dev;
    struct filesys fs;

    fdisk_open(G_DISK_FNAME, &dev);
    filesys_open(&dev, &fs);

    filesys_mkdir(&fs, "/home/pavel/projects/vfat");
    filesys_mkdir(&fs, "/home/pavel/projects/old");
    filesys_mkdir(&fs, "/home/pavel/Desktop");
    filesys_mkdir(&fs, "/home/pavel/Documents");
    filesys_mkdir(&fs, "/home/pavel/Downloads");
    filesys_mkdir(&fs, "/home/pavel/Downloads/astyle");
    filesys_mkdir(&fs, "/home/pavel/Qt");
    filesys_mkdir(&fs, "/home/pavel/Qt/Docs");
    filesys_mkdir(&fs, "/home/pavel/Qt/Examples");
    filesys_mkdir(&fs, "/home/pavel/Qt/Tools");

    filesys_close(&fs);
    filesys_destruct(&fs);

    filesys_open(&dev, &fs);

    struct vdir *dir = filesys_opendir(&fs, "/");
    printdir(&fs, dir, 0);

    filesys_closedir(&fs, dir);
    filesys_close(&fs);
    filesys_destruct(&fs);
    fdisk_close(&dev);
}

MU_TEST_SUITE(filesys_test_suite)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);
    MU_RESET();

    MU_RUN_TEST(test_mkdir);
    MU_RUN_TEST(test_readdir);

    MU_REPORT();
}
