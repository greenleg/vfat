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
    FileDisk *device;

    void SetUp() override
    {
        this->device = new FileDisk("disk0");

        this->device->Create();
        ClusterChainDirectory::FormatDevice(this->device, 1024 * 1024, 512, 1);
    }

    void TearDown() override
    {
        this->device->Close();
        this->device->Delete();
        delete this->device;
        //::__vfat_errno = 0;
    }
};

TEST_F(ClusterChainFileTest, SetLength)
{
    BootSector bootSector;    
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    ClusterChainDirectory *root = new ClusterChainDirectory();
    root->ReadRoot(this->device, &fat);

    DirectoryEntry *e = root->AddFile("index.htm");
    ClusterChainFile *file = ClusterChainDirectory::GetFile(&fat, e);

    ASSERT_EQ(0, file->GetLength());
    file->SetLength(100);
    ASSERT_EQ(100, file->GetLength());
    //ASSERT_GE(file->chain->GetSizeInBytes(), 100);

    delete file;
    delete root;
}

TEST_F(ClusterChainFileTest, ReadWrite)
{
    BootSector bootSector;    
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    ClusterChainDirectory *root = new ClusterChainDirectory();
    root->ReadRoot(this->device, &fat);

    DirectoryEntry *e = root->AddFile("dump.bin");
    ClusterChainFile *file = ClusterChainDirectory::GetFile(&fat, e);

    uint32_t len = 10000;
    uint8_t writeBuf[len];
    uint8_t readBuf[len];

    for (uint32_t i = 0; i < len; i++) {
        writeBuf[i] = i % 256;
    }

    // Write to device
    file->Write(this->device, 0, len, writeBuf);

    // Read from device    
    uint32_t nread = file->Read(this->device, 0, len, readBuf);
    ASSERT_EQ(len, nread);

    for (uint32_t i = 0; i < len; i++) {
        ASSERT_EQ(i % 256, readBuf[i]);
    }

    // Read too long    
    nread = file->Read(this->device, 0, len + 1, readBuf);
    ASSERT_EQ(len, nread);

    delete file;
    delete root;
}
