#include "gtest/gtest.h"
#include "../include/FileDisk.h"
#include "../include/BootSector.h"
#include "../include/Fat.h"

using namespace org::vfat;

class FatTest : public ::testing::Test
{
protected:
    FileDisk *device;

    void SetUp() override
    {
        this->device = new FileDisk("disk0");
        this->device->Create();

        BootSector bootSector;
        bootSector.Create(1024 * 1024, 512, 1);
        bootSector.Write(this->device);

        Fat fat(&bootSector);
        fat.Create();
        fat.Write(this->device);
    }

    void TearDown() override
    {
        this->device->Close();
        device->Delete();

        delete this->device;
    }
};

TEST_F(FatTest, ReadFat)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    ASSERT_EQ(FAT_FIRST_CLUSTER - 1, fat.GetLastAllocatedCluster());

    ASSERT_EQ(FAT_MEDIA_DESCRIPTOR, fat.GetEntry(0));
    ASSERT_EQ(FAT_EOF, fat.GetEntry(1));

    for (uint32_t i = FAT_FIRST_CLUSTER; i < bootSector.GetClusterCount(); i++) {
        ASSERT_EQ(0, fat.GetEntry(i));
    }

    /* TODO: Additional checks */
}

TEST_F(FatTest, AllocateCluster)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    uint32_t newCluster = fat.AllocateChain(1);
    ASSERT_EQ(newCluster, fat.GetLastAllocatedCluster());
}

TEST_F(FatTest, GetFreeClusterCount)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    ASSERT_EQ(bootSector.GetClusterCount() - FAT_FIRST_CLUSTER, fat.GetFreeClusterCount());
}

TEST_F(FatTest, GetFreeClusterCount2)
{
    BootSector bootSector;    
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    uint32_t cluster;
    uint32_t max = fat.GetFreeClusterCount();
    for (uint32_t i = max; i > 0; --i) {
        ASSERT_EQ(i, fat.GetFreeClusterCount());
        cluster = fat.AllocateChain(1);
    }

    ASSERT_EQ(0, fat.GetFreeClusterCount());

    /* Allocated too many clusters */    
    bool errorWasThrown = false;
    try {
        cluster = fat.AllocateChain(1);
    } catch (std::runtime_error error) {
        // FAT is full;
        errorWasThrown = true;
    }

    ASSERT_TRUE(errorWasThrown);
}
