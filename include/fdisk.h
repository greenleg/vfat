#ifndef VFAT_FDISK_H
#define VFAT_FDISK_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include "common.h"

struct fdisk
{
    int fd;
    bool closed;
};

void fdisk_create(/*in*/ const char* fname, /*out*/ struct fdisk *disk);
void fdisk_open(/*in*/ const char* fname, /*out*/ struct fdisk *disk);
void fdisk_close(/*in*/ struct fdisk *disk);
void fdisk_read(/*in*/ struct fdisk *disk, /*in*/ u8 *buf, /*in*/ off_t offset, /*in*/ size_t count);
void fdisk_write(/*in*/ struct fdisk *disk, /*in*/ u8 *buf, /*in*/ off_t offset, /*in*/ size_t count);

#endif /* VFAT_FDISK_H */
