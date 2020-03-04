#include <assert.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <ios>
#include <iostream>
#include <sstream>

#include "../include/common.h"
#include "../include/FileDisk.h"

//void fdisk_create(/*in*/ const char* fname, /*out*/ struct fdisk *disk)
//{
//    int fd = creat(fname, S_IRUSR | S_IWUSR);
//    disk->fd = fd;
//    disk->closed = false;
//}

//void fdisk_open(const char* fname, struct fdisk *disk)
//{
//    int fd = open(fname, O_RDWR, S_IRUSR | S_IWUSR);
//    disk->fd = fd;
//    disk->closed = false;
//}

//void fdisk_close(struct fdisk *disk)
//{
//    close(disk->fd);
//    disk->closed = true;
//}

//void fdisk_read(struct fdisk *disk, u8 *buf, off_t offset, size_t count)
//{
//    assert(!disk->closed);
//    lseek(disk->fd, offset, SEEK_SET);
//    read(disk->fd, buf, count);
//}

//void fdisk_write(struct fdisk *disk, u8 *buf, off_t offset, size_t count)
//{
//    assert(!disk->closed);
//    lseek(disk->fd, offset, SEEK_SET);
//    write(disk->fd, buf, count);
//}

/**
 * @brief Initializes a new instance of the org::vfat::FileDisk.
 * @param fileName
 */
org::vfat::FileDisk::FileDisk(const char *fileName)
{
    this->fileName = fileName;
    this->filePtr = nullptr;
}

org::vfat::FileDisk::~FileDisk()
{
    this->Close();
}

void org::vfat::FileDisk::Create()
{
    this->filePtr = fopen(this->fileName, "w+b");
    if (this->filePtr == nullptr) {
        std::ostringstream msgStream;
        msgStream << "Couldn't create the file '" << this->fileName << "'";
        std::error_code errorCode(errno, std::generic_category());
        throw std::ios_base::failure(msgStream.str(), errorCode);
    }
}

void org::vfat::FileDisk::Open()
{
    this->filePtr = fopen(this->fileName, "r+b");
    if (this->filePtr == nullptr) {
        std::ostringstream msgStream;
        msgStream << "Couldn't open the file '" << this->fileName << "'";
        std::error_code errorCode(errno, std::generic_category());
        throw std::ios_base::failure(msgStream.str(), errorCode);
    }
}

/**
 * @brief Closes the file and deallocates the buffer.
 */
void org::vfat::FileDisk::Close()
{
    if (this->filePtr != nullptr) {
        fclose(this->filePtr);
        this->filePtr = nullptr;
    }
}

void org::vfat::FileDisk::Read(uint8_t *buffer, long int fileOffset, size_t count)
{
    if (this->filePtr == nullptr) {
        std::ostringstream msgStream;
        msgStream << "Couldn't read data from the file '" << this->fileName << "'";
        throw std::ios_base::failure(msgStream.str());
    }

    fseek(this->filePtr, fileOffset, SEEK_SET);
    fread(buffer, sizeof(char), count, this->filePtr);
}

void org::vfat::FileDisk::Write(uint8_t *buffer, long int fileOffset, size_t count)
{
    if (this->filePtr == nullptr) {
        std::ostringstream msgStream;
        msgStream << "Couldn't write data to the file '" << this->fileName << "'";
        throw std::ios_base::failure(msgStream.str());
    }

    fseek(this->filePtr, fileOffset, SEEK_SET);
    fwrite(buffer, sizeof(char), count, this->filePtr);
}

void org::vfat::FileDisk::Delete()
{
    if (this->filePtr != nullptr) {
        std::ostringstream msgStream;
        msgStream << "Couldn't remove disk '" << this->fileName << "' because it is used by another process.";
        throw std::ios_base::failure(msgStream.str());
    }

    remove(this->fileName);
}

