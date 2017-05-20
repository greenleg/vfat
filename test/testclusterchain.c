#include "common.h"
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
    u32 i;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    cluster_chain_create(&cc, &fat, 0);

    u8 write_buf[1025];
    for (i = 0; i < 1025; ++i) {
        write_buf[i] = i % 256;
    }

    cluster_chain_write_data(&disk, &cc, 0, 1025, write_buf);

    MU_ASSERT_U32_EQ(3, fat_get_chain_length(&fat, cc.start_cluster));

    u8 read_buf[1020];
    cluster_chain_read_data(&disk, &cc, 5, 1020, read_buf);

    for (i = 5; i < 1025; ++i) {
        MU_ASSERT_U32_EQ(i % 256, read_buf[i - 5]);
    }

    fdisk_close(&disk);
}

MU_TEST(test_write_data)
{
    MU_PRINT_TEST_INFO();

    u8 chunk_size = 123;
    u8 writes = 10;

    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cluster_chain cc;
    u32 i;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    cluster_chain_create(&cc, &fat, 0);

    u8 data[chunk_size];
    for (i = 0; i < chunk_size; ++i) {
        data[i] = i;
    }

    for (i = 0; i < writes; ++i) {
        cluster_chain_write_data(&disk, &cc, i * chunk_size, chunk_size, data);
    }

    u8 read_buf[writes * chunk_size];
    cluster_chain_read_data(&disk, &cc, 0, writes * chunk_size, read_buf);

    for (i = 0; i < writes * chunk_size; ++i) {
        MU_ASSERT(i % chunk_size == read_buf[i]);
    }

    fdisk_close(&disk);
}

void test_get_free_cluster_count()
{
    MU_PRINT_TEST_INFO();

    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cluster_chain cc;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    cluster_chain_create(&cc, &fat, 0);

    u32 n = fat_get_free_cluster_count(&fat);

    cluster_chain_set_length(&cc, 1);
    MU_ASSERT_U32_EQ(n - 1, fat_get_free_cluster_count(&fat));

    cluster_chain_set_length(&cc, 10);
    MU_ASSERT_U32_EQ(n - 10, fat_get_free_cluster_count(&fat));

    cluster_chain_set_length(&cc, 0);
    MU_ASSERT_U32_EQ(n, fat_get_free_cluster_count(&fat));

    fdisk_close(&disk);
}

MU_TEST(test_set_size)
{
    MU_PRINT_TEST_INFO();

    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cluster_chain cc;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    cluster_chain_create(&cc, &fat, 0);

    cluster_chain_set_size(&cc, vbr_get_bytes_per_cluster(&br));
    MU_ASSERT_U32_EQ(1, cluster_chain_get_length(&cc));

    cluster_chain_set_size(&cc, vbr_get_bytes_per_cluster(&br) + 1);
    MU_ASSERT_U32_EQ(2, cluster_chain_get_length(&cc));

    cluster_chain_set_size(&cc, 0);
    MU_ASSERT_U32_EQ(0, cluster_chain_get_length(&cc));

    cluster_chain_set_size(&cc, 1);
    MU_ASSERT_U32_EQ(1, cluster_chain_get_length(&cc));

    fdisk_close(&disk);
}

MU_TEST(test_get_length_on_disk)
{
    MU_PRINT_TEST_INFO();

    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cluster_chain cc;

    fdisk_open(G_DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    cluster_chain_create(&cc, &fat, 0);
    cluster_chain_set_size(&cc, vbr_get_bytes_per_cluster(&br));

    MU_ASSERT_U32_EQ(vbr_get_bytes_per_cluster(&br), cluster_chain_get_length_on_disk(&cc));

    fdisk_close(&disk);
}

MU_TEST_SUITE(cluster_chain_test_suite)
{
    MU_SUITE_CONFIGURE(&setup, &teardown);
    MU_RESET();

    MU_RUN_TEST(test_read_data_with_offset);
    MU_RUN_TEST(test_write_data);
    MU_RUN_TEST(test_get_free_cluster_count);
    MU_RUN_TEST(test_set_size);
    MU_RUN_TEST(test_get_length_on_disk);

    MU_REPORT();
}
