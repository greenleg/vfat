#ifndef VFAT_FDISK_H
#define VFAT_FDISK_H

#include <stdint.h>
#include <sys/types.h>

struct fdisk
{
    int fd;
};

struct fdisk *fdisk_create(const char* fname);
struct fdisk *fdisk_open(const char* fname);
void fdisk_close(struct fdisk *disk);
void fdisk_read(struct fdisk *disk, uint8_t *buf, off_t offset, size_t count);
void fdisk_write(struct fdisk *disk, uint8_t *buf, off_t offset, size_t count);

#endif /* VFAT_FDISK_H */
