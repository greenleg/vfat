#include "gtest/gtest.h"
#include "../include/FileDisk.h"
#include "../include/vbr.h"
#include "../include/fat.h"

using namespace org::vfat;

class FatTest : public ::testing::Test
{
protected:
    FileDisk *device;

    void SetUp() override
    {
        this->device = new FileDisk("disk0");
        struct vbr br;
        struct fat fat;

        this->device->Create();

        vbr_create(&br, 1024 * 1024, 512, 1);
        vbr_write(&br, this->device);

        fat_create(&br, &fat);
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
    struct vbr br;
    struct fat fat;
    u32 i;

    vbr_read(this->device, &br);
    fat_read(this->device, &br, &fat);

    EXPECT_EQ(FAT_FIRST_CLUSTER - 1, fat.last_alloc_cluster);

    EXPECT_EQ(FAT_MEDIA_DESCRIPTOR, fat.entries[0]);
    EXPECT_EQ(FAT_EOF, fat.entries[1]);

    for (i = FAT_FIRST_CLUSTER; i < br.cluster_count; ++i) {
        EXPECT_EQ(0, fat.entries[i]);
    }

    /* TODO: Additional checks */

    fat_destruct(&fat);
}

TEST_F(FatTest, AllocateCluster)
{
    struct vbr br;
    struct fat fat;

    vbr_read(this->device, &br);
    fat_read(this->device, &br, &fat);

    u32 new_cluster;
    EXPECT_TRUE(fat_alloc_chain(&fat, 1, &new_cluster));
    EXPECT_EQ(new_cluster, fat.last_alloc_cluster);

    fat_destruct(&fat);
}

TEST_F(FatTest, GetFreeClusterCount)
{
    struct vbr br;
    struct fat fat;

    vbr_read(this->device, &br);
    fat_read(this->device, &br, &fat);

    EXPECT_EQ(br.cluster_count - FAT_FIRST_CLUSTER, fat_get_free_cluster_count(&fat));

    fat_destruct(&fat);    
}

TEST_F(FatTest, GetFreeClusterCount2)
{
    //struct fdisk disk;
    struct vbr br;
    struct fat fat;
    u32 max;
    u32 i;
    u32 cluster;

    vbr_read(this->device, &br);
    fat_read(this->device, &br, &fat);

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
