#include "gtest/gtest.h"
#include "../include/filesys.h"

class FileSystemTest : public ::testing::Test
{
protected:
    const char *DISK_FNAME = "disk0";

    void SetUp() override
    {
        struct fdisk dev;
        struct filesys fs;

        fdisk_create(DISK_FNAME, &dev);
        filesys_format(&dev, 1024 * 1024, 512, 1, &fs);

        filesys_destruct(&fs);
        fdisk_close(&dev);
    }

    void TearDown() override
    {
        remove(DISK_FNAME);
    }

    static void PrintDirectory(struct filesys *fs, struct vdir *dir, int level)
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
            PrintDirectory(fs, subdir, level + 1);
            filesys_closedir(fs, subdir);
        }
    }
};


TEST_F(FileSystemTest, MakeDirectory)
{
    struct fdisk dev;
    struct filesys fs;

    fdisk_open(DISK_FNAME, &dev);
    filesys_open(&dev, &fs);

    filesys_mkdir(&fs, "/home/pavel/projects/vfat");

    filesys_destruct(&fs);
    fdisk_close(&dev);
}

TEST_F(FileSystemTest, ReadDirectory)
{
    struct fdisk dev;
    struct filesys fs;

    fdisk_open(DISK_FNAME, &dev);
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
    this->PrintDirectory(&fs, dir, 0);

    filesys_closedir(&fs, dir);
    filesys_close(&fs);
    filesys_destruct(&fs);
    fdisk_close(&dev);
}
