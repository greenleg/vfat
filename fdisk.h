#ifndef VFAT_FDISK_H
#define VFAT_FDISK_H

#include <stdint.h>
#include <sys/types.h>

#include "common.h"

struct fdisk
{
    int fd;
};

void fdisk_create(const char* fname, struct fdisk *disk);
void fdisk_open(const char* fname, struct fdisk *disk);
void fdisk_close(struct fdisk *disk);
void fdisk_read(struct fdisk *disk, u8 *buf, off_t offset, size_t count);
void fdisk_write(struct fdisk *disk, u8 *buf, off_t offset, size_t count);

#endif /* VFAT_FDISK_H */
