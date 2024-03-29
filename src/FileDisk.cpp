#include <cstring>
#include <ios>
#include <iostream>
#include <sstream>

#include "../include/FileDisk.h"

/**
 * @brief Initializes a new instance of the org::vfat::FileDisk.
 * @param fileName
 */
org::vfat::FileDisk::FileDisk(const std::string& fileName)
  : fileName(fileName), filePtr(nullptr)
{
}

org::vfat::FileDisk::~FileDisk()
{
    this->Close();
}

void org::vfat::FileDisk::Create()
{
    this->filePtr = fopen(this->fileName.c_str(), "w+b");
    if (this->filePtr == nullptr) {
        std::ostringstream msgStream;
        msgStream << "Couldn't create the file '" << this->fileName << "'";
        std::error_code errorCode(errno, std::generic_category());
        throw std::ios_base::failure(msgStream.str(), errorCode);
    }
}

void org::vfat::FileDisk::Open()
{
    this->filePtr = fopen(this->fileName.c_str(), "r+b");
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

void org::vfat::FileDisk::Read(uint8_t *buffer, long int fileOffset, size_t count) const
{
    if (this->filePtr == nullptr) {
        std::ostringstream msgStream;
        msgStream << "Couldn't read data from the file '" << this->fileName << "'";
        throw std::ios_base::failure(msgStream.str());
    }

    fseek(this->filePtr, fileOffset, SEEK_SET);
    fread(buffer, sizeof(uint8_t), count, this->filePtr);
}

void org::vfat::FileDisk::Write(uint8_t *buffer, long int fileOffset, size_t count) const
{
    if (this->filePtr == nullptr) {
        std::ostringstream msgStream;
        msgStream << "Couldn't write data to the file '" << this->fileName << "'";
        throw std::ios_base::failure(msgStream.str());
    }

    fseek(this->filePtr, fileOffset, SEEK_SET);
    fwrite(buffer, sizeof(uint8_t), count, this->filePtr);
}

void org::vfat::FileDisk::Delete()
{
    if (this->filePtr != nullptr) {
        std::ostringstream msgStream;
        msgStream << "Couldn't remove disk '" << this->fileName << "' because it is used by another process.";
        throw std::ios_base::failure(msgStream.str());
    }

    remove(this->fileName.c_str());
}

