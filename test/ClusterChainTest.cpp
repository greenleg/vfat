#include "gtest/gtest.h"
#include "../include/common.h"
#include "../include/cch.h"

using namespace org::vfat;

class ClusterChainTest : public ::testing::Test
{
protected:
    FileDisk *device;

    void SetUp() override
    {
        this->device = new FileDisk("disk0");
        BootSector bootSector;
        struct fat fat;

        this->device->Create();

        bootSector.Create(1024 * 1024, 512, 1);
        bootSector.Write(device);

        fat_create(&bootSector, &fat);
        fat_write(&fat, device);

        fat_destruct(&fat);
    }

    void TearDown() override
    {
        this->device->Close();
        device->Delete();

        delete this->device;
    }
};

TEST_F(ClusterChainTest, ReadDataWithOffset)
{
    BootSector bootSector;
    struct fat fat;
    struct cch cc;
    uint32_t i;

    bootSector.Read(this->device);
    fat_read(this->device, &bootSector, &fat);

    cch_create(&cc, &fat, 0);

    uint8_t write_buf[1025];
    for (i = 0; i < 1025; ++i) {
        write_buf[i] = i % 256;
    }

    cch_writedata(this->device, &cc, 0, 1025, write_buf);

    ASSERT_EQ(3, fat_getchainlen(&fat, cc.start_cluster));

    uint8_t read_buf[1020];
    cch_readdata(this->device, &cc, 5, 1020, read_buf);

    for (i = 5; i < 1025; ++i) {
        ASSERT_EQ(i % 256, read_buf[i - 5]);
    }

    fat_destruct(&fat);
}

TEST_F(ClusterChainTest, WriteData)
{
    uint8_t chunk_size = 123;
    uint8_t writes = 10;

    BootSector bootSector;
    struct fat fat;
    struct cch cc;
    uint32_t i;

    bootSector.Read(this->device);
    fat_read(this->device, &bootSector, &fat);

    cch_create(&cc, &fat, 0);

    uint8_t data[chunk_size];
    for (i = 0; i < chunk_size; ++i) {
        data[i] = i;
    }

    for (i = 0; i < writes; ++i) {
        cch_writedata(this->device, &cc, i * chunk_size, chunk_size, data);
    }

    uint8_t read_buf[writes * chunk_size];
    cch_readdata(this->device, &cc, 0, writes * chunk_size, read_buf);

    for (i = 0; i < writes * chunk_size; ++i) {
        EXPECT_EQ(i % chunk_size, read_buf[i]);
    }

    fat_destruct(&fat);
}

TEST_F(ClusterChainTest, GetFreeClusterCount)
{
    BootSector bootSector;
    struct fat fat;
    struct cch cc;

    bootSector.Read(this->device);
    fat_read(this->device, &bootSector, &fat);

    cch_create(&cc, &fat, 0);

    uint32_t n = fat_get_free_cluster_count(&fat);

    cch_setlen(&cc, 1);
    EXPECT_EQ(n - 1, fat_get_free_cluster_count(&fat));

    cch_setlen(&cc, 10);
    EXPECT_EQ(n - 10, fat_get_free_cluster_count(&fat));

    cch_setlen(&cc, 0);
    EXPECT_EQ(n, fat_get_free_cluster_count(&fat));

    fat_destruct(&fat);
}

TEST_F(ClusterChainTest, SetSize)
{
    BootSector bootSector;
    struct fat fat;
    struct cch cc;

    bootSector.Read(this->device);
    fat_read(this->device, &bootSector, &fat);

    cch_create(&cc, &fat, 0);

    cch_setsize(&cc, bootSector.GetBytesPerCluster());
    EXPECT_EQ(1, cch_getlen(&cc));

    cch_setsize(&cc, bootSector.GetBytesPerCluster() + 1);
    EXPECT_EQ(2, cch_getlen(&cc));

    cch_setsize(&cc, 0);
    EXPECT_EQ(0, cch_getlen(&cc));

    cch_setsize(&cc, 1);
    EXPECT_EQ(1, cch_getlen(&cc));

    fat_destruct(&fat);
}

TEST_F(ClusterChainTest, GetSize)
{
    BootSector bootSector;
    struct fat fat;
    struct cch cc;

    bootSector.Read(this->device);
    fat_read(this->device, &bootSector, &fat);

    cch_create(&cc, &fat, 0);
    cch_setsize(&cc, bootSector.GetBytesPerCluster());

    EXPECT_EQ(bootSector.GetBytesPerCluster(), cch_getsize(&cc));

    fat_destruct(&fat);
}
