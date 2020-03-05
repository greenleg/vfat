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
        struct fat fat;

        this->device->Create();

        bootSector.Create(1024 * 1024, 512, 1);
        bootSector.Write(this->device);

        fat_create(&bootSector, &fat);
        fat_write(&fat, this->device);

        fat_destruct(&fat);
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
    struct fat fat;
    uint32_t i;

    bootSector.Read(this->device);
    fat_read(this->device, &bootSector, &fat);

    EXPECT_EQ(FAT_FIRST_CLUSTER - 1, fat.last_alloc_cluster);

    EXPECT_EQ(FAT_MEDIA_DESCRIPTOR, fat.entries[0]);
    EXPECT_EQ(FAT_EOF, fat.entries[1]);

    for (i = FAT_FIRST_CLUSTER; i < bootSector.GetClusterCount(); ++i) {
        EXPECT_EQ(0, fat.entries[i]);
    }

    /* TODO: Additional checks */

    fat_destruct(&fat);
}

TEST_F(FatTest, AllocateCluster)
{
    BootSector bootSector;
    struct fat fat;

    bootSector.Read(this->device);
    fat_read(this->device, &bootSector, &fat);

    uint32_t new_cluster;
    EXPECT_TRUE(fat_alloc_chain(&fat, 1, &new_cluster));
    EXPECT_EQ(new_cluster, fat.last_alloc_cluster);

    fat_destruct(&fat);
}

TEST_F(FatTest, GetFreeClusterCount)
{
    BootSector bootSector;
    struct fat fat;

    bootSector.Read(this->device);
    fat_read(this->device, &bootSector, &fat);

    EXPECT_EQ(bootSector.GetClusterCount() - FAT_FIRST_CLUSTER, fat_get_free_cluster_count(&fat));

    fat_destruct(&fat);    
}

TEST_F(FatTest, GetFreeClusterCount2)
{
    BootSector bootSector;
    struct fat fat;
    uint32_t max;
    uint32_t i;
    uint32_t cluster;

    bootSector.Read(this->device);
    fat_read(this->device, &bootSector, &fat);

    max = fat_get_free_cluster_count(&fat);
    for (i = max; i > 0; --i) {
        EXPECT_EQ(i, fat_get_free_cluster_count(&fat));
        EXPECT_TRUE(fat_alloc_chain(&fat, 1, &cluster));
    }

    EXPECT_EQ(0, fat_get_free_cluster_count(&fat));

    /* Allocated too many clusters */
    EXPECT_FALSE(fat_alloc_chain(&fat, 1, &cluster));
    EXPECT_EQ(EFATFULL, ::__vfat_errno);

    fat_destruct(&fat);
}
