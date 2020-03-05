#include <stdlib.h>

#include "gtest/gtest.h"
#include "../include/FileDisk.h"
#include "../include/BootSector.h"

class BootSectorTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(BootSectorTest, CreateBootRecord)
{
    org::vfat::BootSector bootSector;
    bootSector.Create(1024 * 1024, 512, 1);

    ASSERT_EQ(1024 * 1024, bootSector.GetVolumeSizeInBytes());
    ASSERT_EQ(512,  bootSector.GetBytesPerSector());
    ASSERT_EQ(1,    bootSector.GetSectorsPerCluster());
    ASSERT_EQ(2031, bootSector.GetClusterCount());
    ASSERT_EQ(512,  bootSector.GetFatOffset());
    ASSERT_EQ(8124, bootSector.GetFatSizeInBytes());
    ASSERT_EQ(8704, bootSector.GetClusterHeapOffset());
    ASSERT_EQ(2,    bootSector.GetRootDirFirstCluster());
}

TEST_F(BootSectorTest, CreateAndSaveBootRecord)
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

    ASSERT_EQ(1024 * 1024, bootSector.GetVolumeSizeInBytes());
    ASSERT_EQ(512,  bootSector.GetBytesPerSector());
    ASSERT_EQ(1,    bootSector.GetSectorsPerCluster());
    ASSERT_EQ(2031, bootSector.GetClusterCount());
    ASSERT_EQ(512,  bootSector.GetFatOffset());
    ASSERT_EQ(8124, bootSector.GetFatSizeInBytes());
    ASSERT_EQ(8704, bootSector.GetClusterHeapOffset());
    ASSERT_EQ(2,    bootSector.GetRootDirFirstCluster());

    device.Close();
    device.Delete();
}
