#include <stdlib.h>

#include "fdisk.h"
#include "vbr.h"
#include "minunit.h"
#include "testsuite.h"

MU_TEST(test_vbr_format)
{
    MU_PRINT_TEST_INFO();

    struct vbr *br = malloc(sizeof(struct vbr));
    vbr_format(br, 1024 * 1024, 512, 1);

    MU_ASSERT_INT_EQ(2048, br->volume_length);
    MU_ASSERT_INT_EQ(512,  vbr_get_bytes_per_sector(br));
    MU_ASSERT_INT_EQ(1,    vbr_get_sector_per_cluster(br));
    MU_ASSERT_INT_EQ(2031, br->cluster_count);
    MU_ASSERT_INT_EQ(1,    br->fat_offset);
    MU_ASSERT_INT_EQ(16,   br->fat_length);
    MU_ASSERT_INT_EQ(17,   br->cluster_heap_offset);
    MU_ASSERT_INT_EQ(2,    br->root_dir_first_cluster);

    free(br);
}

MU_TEST(test_vbr_format_and_save)
{
    MU_PRINT_TEST_INFO();

    struct vbr *br = malloc(sizeof(struct vbr));
    vbr_format(br, 1024 * 1024, 512, 1);

    struct sector *sect = sector_create(512);
    vbr_write(br, sect);

    free(br);

    const char* disk_fname = "/home/pavel/projects/vfat/test/disk0";

    /* Write a sector to the disk */
    struct fdisk *disk = fdisk_create(disk_fname);

    fdisk_write(disk, sect->data, 0, sect->size);

    free(sect->data);
    free(sect);

    fdisk_close(disk);
    free(disk);

    disk = fdisk_open(disk_fname);
    sect = sector_create(512);
    fdisk_read(disk, sect->data, 0, sect->size);

    br = malloc(sizeof(struct vbr));
    vbr_read(sect, br);

    MU_ASSERT_INT_EQ(2048, br->volume_length);
    MU_ASSERT_INT_EQ(512,  vbr_get_bytes_per_sector(br));
    MU_ASSERT_INT_EQ(1,    vbr_get_sector_per_cluster(br));
    MU_ASSERT_INT_EQ(2031, br->cluster_count);
    MU_ASSERT_INT_EQ(1,    br->fat_offset);
    MU_ASSERT_INT_EQ(16,   br->fat_length);
    MU_ASSERT_INT_EQ(17,   br->cluster_heap_offset);
    MU_ASSERT_INT_EQ(2,    br->root_dir_first_cluster);

    free(br);

    free(sect->data);
    free(sect);

    fdisk_close(disk);
    free(disk);
}

void run_vbr_suite()
{
    MU_RUN_TEST(test_vbr_format);
    MU_RUN_TEST(test_vbr_format_and_save);
}
