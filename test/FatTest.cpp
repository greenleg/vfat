#include "gtest/gtest.h"
#include "../include/fdisk.h"
#include "../include/vbr.h"
#include "../include/fat.h"

class FatTest : public ::testing::Test
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

TEST_F(FatTest, ReadFat)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    u32 i;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    EXPECT_EQ(FAT_FIRST_CLUSTER - 1, fat.last_alloc_cluster);

    EXPECT_EQ(FAT_MEDIA_DESCRIPTOR, fat.entries[0]);
    EXPECT_EQ(FAT_EOF, fat.entries[1]);

    for (i = FAT_FIRST_CLUSTER; i < br.cluster_count; ++i) {
        EXPECT_EQ(0, fat.entries[i]);
    }

    /* TODO: Additional checks */

    fat_destruct(&fat);
    fdisk_close(&disk);
}

TEST_F(FatTest, AllocateCluster)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    u32 new_cluster;
    EXPECT_TRUE(fat_alloc_chain(&fat, 1, &new_cluster));
    EXPECT_EQ(new_cluster, fat.last_alloc_cluster);

    fat_destruct(&fat);
    fdisk_close(&disk);
}

TEST_F(FatTest, GetFreeClusterCount)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

    EXPECT_EQ(br.cluster_count - FAT_FIRST_CLUSTER, fat_get_free_cluster_count(&fat));

    fat_destruct(&fat);
    fdisk_close(&disk);
}

TEST_F(FatTest, GetFreeClusterCount2)
{
    struct fdisk disk;
    struct vbr br;
    struct fat fat;
    u32 max;
    u32 i;
    u32 cluster;

    fdisk_open(DISK_FNAME, &disk);
    vbr_read(&disk, &br);
    fat_read(&disk, &br, &fat);

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
    fdisk_close(&disk);
}
