#include "minunit.h"
#include "testsuite.h"
#include "../include/fdisk.h"
#include "../include/vbr.h"
#include "../include/fat.h"

/* These variables reside in the process memory permanently */
static const char *G_DISK_FNAME = "/home/pavel/projects/vfat/test/disk0";

MU_TEST_SETUP(setup)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;

    fdisk_create(G_DISK_FNAME, &disk);

    vbr_create(&br, 1024 * 1024, 512, 1);
    vbr_write(&br, &disk);

    fat_create(&br, &fat);
    fat_write(&fat, &disk);

    fdisk_close(&disk);
}

MU_TEST_TEARDOWN(teardown)
{
    remove(G_DISK_FNAME);
}

MU_TEST(test_fat_read)
{
    MU_PRINT_TEST_INFO();

    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    u32 i;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    MU_ASSERT_INT_EQ(FAT_FIRST_CLUSTER - 1, fat.last_alloc_cluster);

    MU_ASSERT_U32_EQ(FAT_MEDIA_DESCRIPTOR, fat.entries[0]);
    MU_ASSERT_U32_EQ(FAT_EOF, fat.entries[1]);

    for (i = FAT_FIRST_CLUSTER; i < br.cluster_count; ++i) {
        MU_ASSERT_U32_EQ(0, fat.entries[i]);
    }

    /* TODO: Additional checks */

    fat_destruct(&fat);
    fdisk_close(&disk);
}

MU_TEST(test_fat_alloc_cluster)
{
    MU_PRINT_TEST_INFO();

    struct fdisk disk;
    struct vbr br;
    struct fat fat;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    u32 new_cluster;
    MU_ASSERT(fat_alloc_chain(&fat, 1, &new_cluster) == true);
    MU_ASSERT_U32_EQ(new_cluster, fat.last_alloc_cluster);

    fat_destruct(&fat);
    fdisk_close(&disk);
}

MU_TEST(test_fat_get_free_cluster_count)
{
    MU_PRINT_TEST_INFO();

    struct fdisk disk;
    struct vbr br;
    struct fat fat;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    MU_ASSERT_U32_EQ(br.cluster_count - FAT_FIRST_CLUSTER, fat_get_free_cluster_count(&fat));

    fat_destruct(&fat);
    fdisk_close(&disk);
}

MU_TEST(test_fat_get_free_cluster_count2)
{
    MU_PRINT_TEST_INFO();

    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    u32 max;
    u32 i;
    u32 cluster;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    max = fat_get_free_cluster_count(&fat);
    for (i = max; i > 0; --i) {
        MU_ASSERT_U32_EQ(i, fat_get_free_cluster_count(&fat));
        MU_ASSERT(fat_alloc_chain(&fat, 1, &cluster) == true);
    }

    MU_ASSERT_U32_EQ(0, fat_get_free_cluster_count(&fat));

    /* Allocated too many clusters */
    MU_ASSERT(fat_alloc_chain(&fat, 1, &cluster) == false);
    MU_ASSERT_INT_EQ(EFATFULL, ::__vfat_errno);

    fat_destruct(&fat);
    fdisk_close(&disk);
}

MU_TEST_SUITE(fat_test_suite)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);
    MU_RESET();

    MU_RUN_TEST(test_fat_read);    
    MU_RUN_TEST(test_fat_alloc_cluster);
    MU_RUN_TEST(test_fat_get_free_cluster_count);
    MU_RUN_TEST(test_fat_get_free_cluster_count2);

    MU_REPORT();
}
