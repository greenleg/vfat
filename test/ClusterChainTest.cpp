#include "gtest/gtest.h"
#include "../include/common.h"
#include "../include/cch.h"

class ClusterChainTest : public ::testing::Test
{
protected:
    const char *DISK_FNAME = "disk0";

    void SetUp() override
    {
        struct fdisk disk;
        struct vbr br;
        struct fat fat;

        fdisk_create(DISK_FNAME, &disk);

        vbr_create(&br, 1024 * 1024, 512, 1);
        vbr_write(&br, &disk);

        fat_create(&br, &fat);
        fat_write(&fat, &disk);

        fdisk_close(&disk);
    }

    void TearDown() override
    {
        remove(DISK_FNAME);
    }
};

TEST_F(ClusterChainTest, ReadDataWithOffset)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cch cc;
    u32 i;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    cch_create(&cc, &fat, 0);

    u8 write_buf[1025];
    for (i = 0; i < 1025; ++i) {
        write_buf[i] = i % 256;
    }

    cch_writedata(&disk, &cc, 0, 1025, write_buf);

    EXPECT_EQ(3, fat_getchainlen(&fat, cc.start_cluster));

    u8 read_buf[1020];
    cch_readdata(&disk, &cc, 5, 1020, read_buf);

    for (i = 5; i < 1025; ++i) {
        EXPECT_EQ(i % 256, read_buf[i - 5]);
    }

    fdisk_close(&disk);
}

TEST_F(ClusterChainTest, WriteData)
{
    u8 chunk_size = 123;
    u8 writes = 10;

    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cch cc;
    u32 i;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    cch_create(&cc, &fat, 0);

    u8 data[chunk_size];
    for (i = 0; i < chunk_size; ++i) {
        data[i] = i;
    }

    for (i = 0; i < writes; ++i) {
        cch_writedata(&disk, &cc, i * chunk_size, chunk_size, data);
    }

    u8 read_buf[writes * chunk_size];
    cch_readdata(&disk, &cc, 0, writes * chunk_size, read_buf);

    for (i = 0; i < writes * chunk_size; ++i) {
        EXPECT_EQ(i % chunk_size, read_buf[i]);
    }

    fdisk_close(&disk);
}

TEST_F(ClusterChainTest, GetFreeClusterCount)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cch cc;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    cch_create(&cc, &fat, 0);

    u32 n = fat_get_free_cluster_count(&fat);

    cch_setlen(&cc, 1);
    EXPECT_EQ(n - 1, fat_get_free_cluster_count(&fat));

    cch_setlen(&cc, 10);
    EXPECT_EQ(n - 10, fat_get_free_cluster_count(&fat));

    cch_setlen(&cc, 0);
    EXPECT_EQ(n, fat_get_free_cluster_count(&fat));

    fdisk_close(&disk);
}

TEST_F(ClusterChainTest, SetSize)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cch cc;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    cch_create(&cc, &fat, 0);

    cch_setsize(&cc, vbr_get_bytes_per_cluster(&br));
    EXPECT_EQ(1, cch_getlen(&cc));

    cch_setsize(&cc, vbr_get_bytes_per_cluster(&br) + 1);
    EXPECT_EQ(2, cch_getlen(&cc));

    cch_setsize(&cc, 0);
    EXPECT_EQ(0, cch_getlen(&cc));

    cch_setsize(&cc, 1);
    EXPECT_EQ(1, cch_getlen(&cc));

    fdisk_close(&disk);
}

TEST_F(ClusterChainTest, GetSize)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    struct cch cc;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    cch_create(&cc, &fat, 0);
    cch_setsize(&cc, vbr_get_bytes_per_cluster(&br));

    EXPECT_EQ(vbr_get_bytes_per_cluster(&br), cch_getsize(&cc));

    fdisk_close(&disk);
}
