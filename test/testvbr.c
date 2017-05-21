#include <stdlib.h>

#include "fdisk.h"
#include "vbr.h"
#include "minunit.h"
#include "testsuite.h"

MU_TEST(test_vbr_create)
{
    MU_PRINT_TEST_INFO();

    struct vbr br;
    vbr_create(&br, 1024 * 1024, 512, 1);

    MU_ASSERT_INT_EQ(2048, br.volume_length);
    MU_ASSERT_INT_EQ(512,  vbr_get_bytes_per_sector(&br));
    MU_ASSERT_INT_EQ(1,    vbr_get_sectors_per_cluster(&br));
    MU_ASSERT_INT_EQ(2031, br.cluster_count);
    MU_ASSERT_INT_EQ(1,    br.fat_offset);
    MU_ASSERT_INT_EQ(16,   br.fat_length);
    MU_ASSERT_INT_EQ(17,   br.cluster_heap_offset);
    MU_ASSERT_INT_EQ(2,    br.rootdir_first_cluster);
}

MU_TEST(test_vbr_create_and_save)
{
    MU_PRINT_TEST_INFO();

    const char* disk_fname = "/home/pavel/projects/vfat/test/disk0";
    struct fdisk disk;
    struct vbr br;

    fdisk_create(disk_fname, &disk);
    vbr_create(&br, 1024 * 1024, 512, 1);
    vbr_write(&br, &disk);

    fdisk_close(&disk);

    fdisk_open(disk_fname, &disk);
    vbr_read(&disk, &br);

    MU_ASSERT_INT_EQ(2048, br.volume_length);
    MU_ASSERT_INT_EQ(512,  vbr_get_bytes_per_sector(&br));
    MU_ASSERT_INT_EQ(1,    vbr_get_sectors_per_cluster(&br));
    MU_ASSERT_INT_EQ(2031, br.cluster_count);
    MU_ASSERT_INT_EQ(1,    br.fat_offset);
    MU_ASSERT_INT_EQ(16,   br.fat_length);
    MU_ASSERT_INT_EQ(17,   br.cluster_heap_offset);
    MU_ASSERT_INT_EQ(2,    br.rootdir_first_cluster);

    fdisk_close(&disk);
    remove(disk_fname);
}

MU_TEST_SUITE(vbr_test_suite)
{
    MU_RESET();

    MU_RUN_TEST(test_vbr_create);
    MU_RUN_TEST(test_vbr_create_and_save);

    MU_REPORT();
}
