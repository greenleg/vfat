#include "gtest/gtest.h"
#include "../include/ClusterChain.h"
#include "../include/ClusterChainDirectory.h"
#include "../include/DirectoryEntry.h"

using namespace org::vfat;

class ClusterChainDirectoryTest : public ::testing::Test
{
protected:
    FileDisk device;
    
    ClusterChainDirectoryTest() : device("disk0") {}
    
    void FormatDevice(Device& device, uint64_t volumeSize, uint16_t bytesPerSector, uint16_t sectorPerCluster)
    {
        BootSector bootSector;
        bootSector.Create(volumeSize, bytesPerSector, sectorPerCluster);
        bootSector.Write(device);

        Fat fat(bootSector);
        fat.Create();

        ClusterChainDirectory root;
        root.CreateRoot(fat);

        root.Write(device, fat);
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

TEST_F(ClusterChainDirectoryTest, AddEntry)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChainDirectory root;
    root.ReadRoot(this->device, fat);

    ASSERT_EQ(0, root.GetEntries().size());

    DirectoryEntry e;
    root.AddEntry(e, fat);
    ASSERT_EQ(1, root.GetEntries().size());
    ASSERT_NE(0, bootSector.GetRootDirFirstCluster());
    //ASSERT_EQ(root.capacity, root->chain->GetSizeInBytes() / FAT_DIR_ENTRY_SIZE);
}

TEST_F(ClusterChainDirectoryTest, AddRemoveEntries)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChainDirectory root;
    root.ReadRoot(this->device, fat);

    ASSERT_EQ(0, root.GetEntries().size());

    for (uint32_t i = 0; i < 100; ++i) {
        DirectoryEntry e;
        root.AddEntry(e, fat);
    }

    for (uint32_t i = 0; i < 100; ++i) {
        ASSERT_EQ(100 - i, root.GetEntries().size());
        root.RemoveEntry(fat, 0);
    }

    ASSERT_EQ(0, root.GetEntries().size());
}

TEST_F(ClusterChainDirectoryTest, AddSubDirectory)
{
    BootSector bootSector;    
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChainDirectory root;
    root.ReadRoot(this->device, fat);

    const std::string name = "A nice directory";

    DirectoryEntry e1 = root.AddDirectory(name, this->device, fat);
    DirectoryEntry e2 = root.FindEntry(name);

    std::string entryName = e1.GetName();
    ASSERT_EQ(name, entryName);

    entryName = e2.GetName();
    ASSERT_EQ(name, entryName);
}

TEST_F(ClusterChainDirectoryTest, AddTooManyDirectories)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChainDirectory root;
    root.ReadRoot(this->device, fat);

    char nameBuf[255];
    uint32_t count = 0;

    do {
        uint32_t freeBeforeAdd = fat.GetFreeClusterCount();
        sprintf(nameBuf, "this is test directory with index %d", count++);
        //bool errorWasThrown = false;
        try {
            DirectoryEntry e = root.AddDirectory(nameBuf, this->device, fat);
        } catch (std::runtime_error err) {
            ASSERT_EQ(freeBeforeAdd, fat.GetFreeClusterCount());
            break;
        }
    } while (true);
}

TEST_F(ClusterChainDirectoryTest, RemoveDirectory)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChainDirectory root;
    root.ReadRoot(this->device, fat);

    uint32_t freeBefore = fat.GetFreeClusterCount();
    uint32_t entriesBefore = root.GetEntries().size();

    const std::string& dirName = "testdir";
    root.AddDirectory(dirName, this->device, fat);
    ASSERT_EQ(freeBefore - 1, fat.GetFreeClusterCount());
    ASSERT_EQ(entriesBefore + 1, root.GetEntries().size());
    ASSERT_NE(-1, root.FindEntryIndex(dirName));

    root.RemoveDirectory(dirName, this->device, fat);
    ASSERT_EQ(freeBefore, fat.GetFreeClusterCount());
    ASSERT_EQ(entriesBefore, root.GetEntries().size());
    ASSERT_EQ(-1, root.FindEntryIndex(dirName));
}

TEST_F(ClusterChainDirectoryTest, UniqueDirectoryName)
{
    BootSector bootSector;    
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChainDirectory root;
    root.ReadRoot(this->device, fat);

    root.AddDirectory("home", this->device, fat);
    bool errorWasThrown = false;
    try {
        DirectoryEntry e = root.AddDirectory("home", this->device, fat);
    } catch (std::runtime_error error) {
        errorWasThrown = true;
    }

    ASSERT_TRUE(errorWasThrown);

    // Reuse name
    root.RemoveDirectory("home", this->device, fat);
    root.AddDirectory("home", this->device, fat);
}

TEST_F(ClusterChainDirectoryTest, RenameFile)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChainDirectory root;
    root.ReadRoot(this->device, fat);

    DirectoryEntry e = root.AddFile("oldfile", this->device, fat);

    ASSERT_NE(-1, root.FindEntryIndex("oldfile"));
    ASSERT_EQ(-1, root.FindEntryIndex("newfile"));

    root.SetName(this->device, fat, e, "newfile");

    ASSERT_EQ(-1, root.FindEntryIndex("oldfile"));
    ASSERT_NE(-1, root.FindEntryIndex("newfile"));
}

TEST_F(ClusterChainDirectoryTest, MoveFile)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChainDirectory root;
    root.ReadRoot(this->device, fat);

    DirectoryEntry de = root.AddDirectory("home", this->device, fat);
    DirectoryEntry fe = root.AddFile("dump.bin", this->device, fat);

    ClusterChainDirectory dir = ClusterChainDirectory::GetDirectory(this->device, fat, de);
    root.Move(this->device, fat, fe, dir, "dump2.bin");    

    ASSERT_EQ(1, root.GetEntries().size());
    ASSERT_EQ(3, dir.GetEntries().size());  // including "." and ".." directories.    
    ASSERT_NE(-1, root.FindEntryIndex("home"));
    ASSERT_EQ(-1, root.FindEntryIndex("dump.bin"));
    ASSERT_EQ(-1, root.FindEntryIndex("dump2.bin"));
    ASSERT_NE(-1, dir.FindEntryIndex("dump2.bin"));
}

TEST_F(ClusterChainDirectoryTest, MoveDirectory)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChainDirectory root;
    root.ReadRoot(this->device, fat);

    DirectoryEntry de1 = root.AddDirectory("dir1", this->device, fat);
    DirectoryEntry de2 = root.AddDirectory("dir2", this->device, fat);

    ClusterChainDirectory dir1 = ClusterChainDirectory::GetDirectory(this->device, fat, de1);
    ClusterChainDirectory dir2 = ClusterChainDirectory::GetDirectory(this->device, fat, de2);

    DirectoryEntry fe1 = dir1.AddFile("dump1.bin", this->device, fat);
    DirectoryEntry fe2 = dir1.AddFile("dump2.bin", this->device, fat);
    DirectoryEntry fe3 = dir1.AddFile("dump3.bin", this->device, fat);

    ClusterChainFile file3 = ClusterChainDirectory::GetFile(fe3);

    ASSERT_EQ(2, root.GetEntries().size());
    ASSERT_EQ(5, dir1.GetEntries().size()); // including "." and ".." directories
    ASSERT_EQ(2, dir2.GetEntries().size());

    root.Move(this->device, fat, de1, dir2, "dir1");

    dir1.Write(this->device, fat);
    dir2.Write(this->device, fat);

    ASSERT_NE(-1, root.FindEntryIndex("dir2"));
    dir2 = ClusterChainDirectory::GetDirectory(this->device, fat, de2);

    ASSERT_NE(-1, dir2.FindEntryIndex("dir1"));
    dir1 = ClusterChainDirectory::GetDirectory(this->device, fat, de1);

    ASSERT_EQ(1, root.GetEntries().size());
    ASSERT_EQ(5, dir1.GetEntries().size());
    ASSERT_EQ(3, dir2.GetEntries().size());
}

TEST_F(ClusterChainDirectoryTest, CopyFile)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChainDirectory root;
    root.ReadRoot(this->device, fat);

    DirectoryEntry de1 = root.AddDirectory("dir1", this->device, fat);
    DirectoryEntry de2 = root.AddDirectory("dir2", this->device, fat);

    ClusterChainDirectory dir1 = ClusterChainDirectory::GetDirectory(this->device, fat, de1);
    ClusterChainDirectory dir2 = ClusterChainDirectory::GetDirectory(this->device, fat, de2);
    DirectoryEntry orige = dir1.AddFile("dump.bin", this->device, fat);

    ASSERT_EQ(2, root.GetEntries().size());
    ASSERT_EQ(3, dir1.GetEntries().size()); // including "." and ".." directories
    ASSERT_EQ(2, dir2.GetEntries().size());

    uint32_t len = 4096 * 4;
    uint8_t buf[len];
    for (uint32_t i = 0; i < len; ++i) {
        buf[i] = i % 256;
    }    

    // Write to source file    
    ClusterChainFile orig = ClusterChainDirectory::GetFile(orige);
    orig.Write(this->device, fat, 0, len, buf);

    orige = dir1.FindEntry("dump.bin");
    orige = orig.GetEntry();

    ClusterChainFile copy = dir1.CopyFile(this->device, fat, orige, dir2);
    
    DirectoryEntry& copye = dir2.FindEntry("dump.bin");

//    // Read from copy
//    std::cout << "Reading from copy..." << std::endl;
//    ClusterChainFile copy = ClusterChainDirectory::GetFile(fat, copye);

    ASSERT_EQ(len, copy.GetLength());
    uint32_t nread = copy.Read(this->device, fat, 0, len, buf);

    for (uint32_t i = 0; i < len; ++i) {
        ASSERT_EQ(i % 256, buf[i]);
    }

    ASSERT_EQ(2, root.GetEntries().size());
    ASSERT_EQ(3, dir1.GetEntries().size());
    ASSERT_EQ(3, dir2.GetEntries().size());
}

TEST_F(ClusterChainDirectoryTest, CopyDirectory)
{
    BootSector bootSector;
    bootSector.Read(this->device);

    Fat fat(bootSector);
    fat.Read(this->device);

    ClusterChainDirectory root;
    root.ReadRoot(this->device, fat);

    DirectoryEntry de1 = root.AddDirectory("dir1", this->device, fat);
    DirectoryEntry de2 = root.AddDirectory("dir2", this->device, fat);
    ClusterChainDirectory dir1 = ClusterChainDirectory::GetDirectory(this->device, fat, de1);
    ClusterChainDirectory dir2 = ClusterChainDirectory::GetDirectory(this->device, fat, de2);

    DirectoryEntry& fe = dir1.AddFile("dump.bin", this->device, fat);

    // Keep in mind that all directories except root include "." and ".." sub-directories
    ASSERT_EQ(2, root.GetEntries().size());
    ASSERT_EQ(3, dir1.GetEntries().size());
    ASSERT_EQ(2, dir2.GetEntries().size());

    uint32_t len = 4096 * 4 + 100;
    uint8_t buf[len];
    for (uint32_t i = 0; i < len; ++i) {
        buf[i] = i % 256;
    }

    // Write to file
    ClusterChainFile file = ClusterChainDirectory::GetFile(fe);
    file.Write(this->device, fat, 0, len, buf);
    fe = file.GetEntry();

    dir1.Write(this->device, fat);
    dir2.Write(this->device, fat);

    root.CopyDirectory(this->device, fat, de1, dir2);

    ASSERT_NE(-1, root.FindEntryIndex("dir1"));
    ASSERT_NE(-1, root.FindEntryIndex("dir2"));
    ASSERT_NE(-1, dir1.FindEntryIndex("dump.bin"));

    ASSERT_EQ(2, root.GetEntries().size());
    ASSERT_EQ(3, dir1.GetEntries().size());
    ASSERT_EQ(3, dir2.GetEntries().size());
    
    DirectoryEntry copyde1 = dir2.FindEntry("dir1");

    ClusterChainDirectory copydir1 = ClusterChainDirectory::GetDirectory(this->device, fat, copyde1);
    DirectoryEntry copyfe = copydir1.FindEntry("dump.bin");

    ASSERT_EQ(3, copydir1.GetEntries().size());

    // Read from copy
    ClusterChainFile copyfile = ClusterChainDirectory::GetFile(copyfe);
    ASSERT_EQ(len, copyfile.GetLength());
    uint32_t nread = copyfile.Read(this->device, fat, 0, len, buf);

    for (uint32_t i = 0; i < len; ++i) {
        ASSERT_EQ(i % 256, buf[i]);
    }

    // Writing to the copy doesn't affect to the origin file.
    buf[0] = 50;
    copyfile.Write(this->device, fat, 0, 1, buf);
    nread = file.Read(this->device, fat, 0, 1, buf);
    ASSERT_EQ(0, buf[0]);
    nread = copyfile.Read(this->device, fat, 0, 1, buf);
    ASSERT_EQ(50, buf[0]);
}
