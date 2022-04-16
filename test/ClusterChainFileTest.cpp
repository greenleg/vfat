#include <errno.h>
#include "gtest/gtest.h"
#include "../include/ClusterChain.h"
#include "../include/ClusterChainDirectory.h"
#include "../include/ClusterChainFile.h"
#include "../include/DirectoryEntry.h"

using namespace org::vfat;

class ClusterChainFileTest : public ::testing::Test
{
protected:
    FileDisk device;
    
    ClusterChainFileTest() : device("disk0") { }
    
    void FormatDevice(Device& device, uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorPerCluster)
    {
        BootSector bootSector;
        bootSector.Create(volumeSize, bytesPerSector, sectorPerCluster);
        bootSector.Write(device);

        Fat fat(bootSector);
        fat.Create();

        ClusterChainDirectory root;
        root.CreateRoot(&fat);

        root.Write(device);
        fat.Write(device);
    }

    void SetUp() override
    {
        this->device.Create();
        FormatDevice(this->device, 1024 * 1024, 512, 1);
    }

    void TearDown() override
    {
        this->device.Close();
        this->device.Delete();
    }
};

TEST_F(ClusterChainFileTest, SetLength)
{
    BootSector bootSector;    
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChainDirectory root;
    root.ReadRoot(this->device, &fat);

    DirectoryEntry e = root.AddFile("index.htm", this->device);
    ClusterChainFile file = ClusterChainDirectory::GetFile(&fat, e);

    ASSERT_EQ(0, file.GetLength());
    file.SetLength(100);
    ASSERT_EQ(100, file.GetLength());
}

TEST_F(ClusterChainFileTest, ReadWrite)
{
    BootSector bootSector;    
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChainDirectory root;
    root.ReadRoot(this->device, &fat);

    DirectoryEntry e = root.AddFile("dump.bin", this->device);
    ClusterChainFile file = ClusterChainDirectory::GetFile(&fat, e);

    uint32_t len = 10000;
    uint8_t writeBuf[len];
    uint8_t readBuf[len];

    for (uint32_t i = 0; i < len; ++i) {
        writeBuf[i] = i % 256;
    }

    // Write to device
    file.Write(this->device, 0, len, writeBuf);

    // Read from device    
    uint32_t nread = file.Read(this->device, 0, len, readBuf);
    ASSERT_EQ(len, nread);

    for (uint32_t i = 0; i < len; ++i) {
        ASSERT_EQ(i % 256, readBuf[i]);
    }

    // Read too long    
    nread = file.Read(this->device, 0, len + 1, readBuf);
    ASSERT_EQ(len, nread);
}
