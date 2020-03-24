#include <errno.h>
#include "gtest/gtest.h"
#include "../include/common.h"
#include "../include/alist.h"
#include "../include/ClusterChain.h"
#include "../include/cchdir.h"
#include "../include/cchfile.h"
#include "../include/lfnde.h"

using namespace org::vfat;

class ClusterChainFileTest : public ::testing::Test
{
protected:
    FileDisk *device;

    void SetUp() override
    {
        this->device = new FileDisk("disk0");

        this->device->Create();
        cchdir_formatdev(this->device, 1024 * 1024, 512, 1);
    }

    void TearDown() override
    {
        this->device->Close();
        this->device->Delete();
        delete this->device;
        ::__vfat_errno = 0;
    }
};

TEST_F(ClusterChainFileTest, SetLength)
{
    BootSector bootSector;    
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    struct cchdir root;
    cchdir_readroot(this->device, &fat, &root);

    struct lfnde e;
    struct cchfile file;

    cchdir_addfile(&root, "index.htm", &e);
    cchdir_getfile(&root, &e, &file);

    EXPECT_EQ(0, cchfile_getlen(&file));
    cchfile_setlen(&file, 100);
    EXPECT_EQ(100, cchfile_getlen(&file));
    EXPECT_GE(file.chain->GetSizeInBytes(), 100);

    cchfile_destruct(&file);
    cchdir_destruct(&root);
    //fat_destruct(&fat);
}

TEST_F(ClusterChainFileTest, ReadWrite)
{
    BootSector bootSector;    
    bootSector.Read(this->device);

    Fat fat(&bootSector);
    fat.Read(this->device);

    struct cchdir root;
    cchdir_readroot(this->device, &fat, &root);

    struct lfnde e;
    struct cchfile file;

    cchdir_addfile(&root, "dump.bin", &e);
    cchdir_getfile(&root, &e, &file);

    uint32_t i, nread;
    uint32_t len = 10000;
    uint8_t writebuf[len];
    uint8_t readbuf[len];

    for (i = 0; i < len; ++i) {
        writebuf[i] = i % 256;
    }

    // Write to device
    cchfile_write(this->device, &file, 0, len, writebuf);

    // Read from device
    cchfile_read(this->device, &file, 0, len, &nread, readbuf);
    EXPECT_EQ(len, nread);

    for (i = 0; i < len; ++i) {
        EXPECT_EQ(i % 256, readbuf[i]);
    }

    // Read too long
    cchfile_read(this->device, &file, 0, len + 1, &nread, readbuf);
    EXPECT_EQ(len, nread);

    cchfile_destruct(&file);
    cchdir_destruct(&root);
    //fat_destruct(&fat);
}
