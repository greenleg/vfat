#include <stdlib.h>

#include "gtest/gtest.h"
#include "../include/FileDisk.h"
#include "../include/BootSector.h"

class BootRecordTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(BootRecordTest, FileDisk)
{
    org::vfat::FileDisk disk("disk0");

    disk.Create();
    disk.Open();
    disk.Close();
}

TEST_F(BootRecordTest, CreateBootRecord)
{
    org::vfat::BootSector bootSector;
    bootSector.Create(1024 * 1024, 512, 1);

    EXPECT_EQ(2048, bootSector.GetVolumeSizeInBytes());
    EXPECT_EQ(512,  bootSector.GetBytesPerSector());
    EXPECT_EQ(1,    bootSector.GetSectorsPerCluster());
    EXPECT_EQ(2031, bootSector.GetClusterCount());
    EXPECT_EQ(1,    bootSector.GetFatOffset());
    EXPECT_EQ(16,   bootSector.GetFatSizeInBytes());
    EXPECT_EQ(17,   bootSector.GetClusterHeapOffset());
    EXPECT_EQ(2,    bootSector.GetRootDirFirstCluster());
}

TEST_F(BootRecordTest, CreateAndSaveBootRecord)
{
    /// TODO: Set a path to a temp directory beyond the repository.    
    org::vfat::FileDisk device("disk0");
    org::vfat::BootSector bootSector;

    device.Create();
    bootSector.Create(1024 * 1024, 512, 1);
    bootSector.Write(&device);
    device.Close();

    device.Open();
    bootSector.Read(&device);

    EXPECT_EQ(2048, bootSector.GetVolumeSizeInBytes());
    EXPECT_EQ(512,  bootSector.GetBytesPerSector());
    EXPECT_EQ(1,    bootSector.GetSectorsPerCluster());
    EXPECT_EQ(2031, bootSector.GetClusterCount());
    EXPECT_EQ(1,    bootSector.GetFatOffset());
    EXPECT_EQ(16,   bootSector.GetFatSizeInBytes());
    EXPECT_EQ(17,   bootSector.GetClusterHeapOffset());
    EXPECT_EQ(2,    bootSector.GetRootDirFirstCluster());

    device.Close();
    device.Delete();
}
