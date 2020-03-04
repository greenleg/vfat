#ifndef VFAT_FILEDISK_H
#define VFAT_FILEDISK_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include "common.h"

//struct fdisk
//{
//    int fd;
//    bool closed;
//};

//void fdisk_create(/*in*/ const char* fname, /*out*/ struct fdisk *disk);
//void fdisk_open(/*in*/ const char* fname, /*out*/ struct fdisk *disk);
//void fdisk_close(/*in*/ struct fdisk *disk);
//void fdisk_read(/*in*/ struct fdisk *disk, /*in*/ u8 *buf, /*in*/ off_t offset, /*in*/ size_t count);
//void fdisk_write(/*in*/ struct fdisk *disk, /*in*/ u8 *buf, /*in*/ off_t offset, /*in*/ size_t count);

namespace org::vfat
{
    class FileDisk
    {
    private:
        const char *fileName;
        FILE *filePtr;

    public:
        FileDisk(const char* fileName);
        ~FileDisk();
        void Create();
        void Open();
        void Close();
        void Delete();
        void Read(uint8_t *buffer, long int fileOffset, size_t count);
        void Write(uint8_t *buffer, long int fileOffset, size_t count);
    };
}

#endif /* VFAT_FILEDISK_H */
