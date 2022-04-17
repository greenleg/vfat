#include "gtest/gtest.h"
#include "../include/ClusterChain.h"

using namespace org::vfat;

class ClusterChainTest : public ::testing::Test
{
protected:
    FileDisk device;
    
    ClusterChainTest() : device("disk0") { }

    void SetUp() override
    {
        this->device.Create();

        BootSector bootSector;
        bootSector.Create(1024 * 1024, 512, 1);
        bootSector.Write(device);

        Fat fat(bootSector);
        fat.Create();
        fat.Write(device);
    }

    void TearDown() override
    {
        this->device.Close();
        this->device.Delete();
    }
};

TEST_F(ClusterChainTest, ReadDataWithOffset)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChain cc(0);

    uint8_t writeBuf[1025];
    for (uint32_t i = 0; i < 1025; ++i) {
        writeBuf[i] = i % 256;
    }

    cc.WriteData(this->device, fat, 0, 1025, writeBuf);

    ASSERT_EQ(3, fat.GetChainLength(cc.GetStartCluster()));

    uint8_t readBuf[1020];
    cc.ReadData(this->device, fat, 5, 1020, readBuf);

    for (uint32_t i = 5; i < 1025; ++i) {
        ASSERT_EQ(i % 256, readBuf[i - 5]);
    }
}

TEST_F(ClusterChainTest, WriteData)
{
    uint8_t chunkSize = 123;
    uint8_t writes = 10;

    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChain cc(0);

    uint8_t data[chunkSize];
    for (uint32_t i = 0; i < chunkSize; ++i) {
        data[i] = i;
    }

    for (uint32_t i = 0; i < writes; ++i) {
        cc.WriteData(this->device, fat, i * chunkSize, chunkSize, data);
    }

    uint8_t readBuf[writes * chunkSize];
    cc.ReadData(this->device, fat, 0, writes * chunkSize, readBuf);

    for (uint32_t i = 0; i < writes * chunkSize; ++i) {
        ASSERT_EQ(i % chunkSize, readBuf[i]);
    }
}

TEST_F(ClusterChainTest, GetFreeClusterCount)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChain cc(0);

    uint32_t n = fat.GetFreeClusterCount();

    cc.SetLength(fat, 1);
    ASSERT_EQ(n - 1, fat.GetFreeClusterCount());

    cc.SetLength(fat, 10);
    ASSERT_EQ(n - 10, fat.GetFreeClusterCount());

    cc.SetLength(fat, 0);
    ASSERT_EQ(n, fat.GetFreeClusterCount());
}

TEST_F(ClusterChainTest, SetSize)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChain cc(0);

    cc.SetSizeInBytes(fat, bootSector.GetBytesPerCluster());
    ASSERT_EQ(1, cc.GetLength(fat));

    cc.SetSizeInBytes(fat, bootSector.GetBytesPerCluster() + 1);
    ASSERT_EQ(2, cc.GetLength(fat));

    cc.SetSizeInBytes(fat, 0);
    ASSERT_EQ(0, cc.GetLength(fat));

    cc.SetSizeInBytes(fat, 1);
    ASSERT_EQ(1, cc.GetLength(fat));
}

TEST_F(ClusterChainTest, GetSize)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChain cc(0);
    cc.SetSizeInBytes(fat, bootSector.GetBytesPerCluster());

    ASSERT_EQ(bootSector.GetBytesPerCluster(), cc.GetSizeInBytes(fat));
}
