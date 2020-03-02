#include <stdlib.h>

#include "gtest/gtest.h"
#include "../include/fdisk.h"
#include "../include/vbr.h"

class BootRecordTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(BootRecordTest, CreateBootRecord)
{
    struct vbr br;
    vbr_create(&br, 1024 * 1024, 512, 1);

    EXPECT_EQ(2048, br.volume_length);
    EXPECT_EQ(512,  vbr_get_bytes_per_sector(&br));
    EXPECT_EQ(1,    vbr_get_sectors_per_cluster(&br));
    EXPECT_EQ(2031, br.cluster_count);
    EXPECT_EQ(1,    br.fat_offset);
    EXPECT_EQ(16,   br.fat_length);
    EXPECT_EQ(17,   br.cluster_heap_offset);
    EXPECT_EQ(2,    br.rootdir_first_cluster);
}

TEST_F(BootRecordTest, CreateAndSaveBootRecord)
{
    /// TODO: Set a path to a temp directory beyond the repository.
    const char* diskFileName = "disk0";
    struct fdisk disk;
    struct vbr br;

    fdisk_create(diskFileName, &disk);
    vbr_create(&br, 1024 * 1024, 512, 1);
    vbr_write(&br, &disk);

    fdisk_close(&disk);

    fdisk_open(diskFileName, &disk);
    vbr_read(&disk, &br);

    EXPECT_EQ(2048, br.volume_length);
    EXPECT_EQ(512,  vbr_get_bytes_per_sector(&br));
    EXPECT_EQ(1,    vbr_get_sectors_per_cluster(&br));
    EXPECT_EQ(2031, br.cluster_count);
    EXPECT_EQ(1,    br.fat_offset);
    EXPECT_EQ(16,   br.fat_length);
    EXPECT_EQ(17,   br.cluster_heap_offset);
    EXPECT_EQ(2,    br.rootdir_first_cluster);

    fdisk_close(&disk);
    remove(diskFileName);
}
