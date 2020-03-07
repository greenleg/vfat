#include "gtest/gtest.h"
#include "../include/common.h"
#include "../include/FileDisk.h"
#include "../include/BootSector.h"
#include "../include/fat.h"

using namespace org::vfat;

class FatTest : public ::testing::Test
{
protected:
    FileDisk *device;

    void SetUp() override
    {
        this->device = new FileDisk("disk0");
        BootSector bootSector;
        Fat fat(&bootSector);

        this->device->Create();

        bootSector.Create(1024 * 1024, 512, 1);
        bootSector.Write(this->device);

        fat.Create();
        fat.Write(this->device);

        //fat_destruct(&fat);
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
    Fat fat(&bootSector);
    uint32_t i;

    bootSector.Read(this->device);
    fat.Read(this->device);

    ASSERT_EQ(FAT_FIRST_CLUSTER - 1, fat.GetLastAllocatedCluster());

    ASSERT_EQ(FAT_MEDIA_DESCRIPTOR, fat.GetEntry(0));
    ASSERT_EQ(FAT_EOF, fat.GetEntry(1));

    for (i = FAT_FIRST_CLUSTER; i < bootSector.GetClusterCount(); ++i) {
        ASSERT_EQ(0, fat.GetEntry(i));
    }

    /* TODO: Additional checks */

    //fat_destruct(&fat);
}

TEST_F(FatTest, AllocateCluster)
{
    BootSector bootSector;
    Fat fat(&bootSector);

    bootSector.Read(this->device);
    fat.Read(this->device);

    uint32_t new_cluster = fat.AllocateChain(1);
    ASSERT_EQ(new_cluster, fat.GetLastAllocatedCluster());

    //fat_destruct(&fat);
}

TEST_F(FatTest, GetFreeClusterCount)
{
    BootSector bootSector;
    Fat fat(&bootSector);

    bootSector.Read(this->device);
    fat.Read(this->device);

    ASSERT_EQ(bootSector.GetClusterCount() - FAT_FIRST_CLUSTER, fat.GetFreeClusterCount());

    //fat_destruct(&fat);
}

TEST_F(FatTest, GetFreeClusterCount2)
{
    BootSector bootSector;
    Fat fat(&bootSector);
    uint32_t max;
    uint32_t i;
    uint32_t cluster;

    bootSector.Read(this->device);
    fat.Read(this->device);

    max = fat.GetFreeClusterCount();
    for (i = max; i > 0; --i) {
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
        // EXPECT_EQ(EFATFULL, ::__vfat_errno);
        errorWasThrown = true;
    }

    ASSERT_TRUE(errorWasThrown);
}
