#include "minunit.h"
#include "clusterchain.h"

static const char*  G_DISK_FNAME = "/home/pavel/projects/vfat/test/disk0";

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

MU_TEST(test_read_data_with_offset)
{
    MU_PRINT_TEST_INFO();

    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cluster_chain cc;
    uint32_t i;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    cc.fat = &fat;
    cc.start_cluster = 0;

    cluster_chain_set_length(&cc, 1);

    uint8_t write_buf[1025];
    for (i = 0; i < 1025; ++i) {
        write_buf[i] = i % 256;
    }

    cluster_chain_write_data(&disk, &cc, 0, 1025, write_buf);

    MU_ASSERT_U32_EQ(3, fat_get_chain_length(&fat, cc.start_cluster));

    uint8_t read_buf[1020];
    cluster_chain_read_data(&disk, &cc, 5, 1020, read_buf);

    for (i = 5; i < 1025; ++i) {
        MU_ASSERT_U32_EQ(i % 256, read_buf[i - 5]);
    }
}

MU_TEST_SUITE(cluster_chain_test_suite)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);

    MU_RUN_TEST(test_read_data_with_offset);
}
