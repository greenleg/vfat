#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "fdisk.h"

struct fdisk *fdisk_create(const char* fname)
{
    int fd = creat(fname, S_IRUSR | S_IWUSR);
    struct fdisk *disk = malloc(sizeof(struct fdisk));
    disk->fd = fd;
    return disk;
}

struct fdisk *fdisk_open(const char* fname)
{
    int fd = open(fname, O_RDWR, S_IRUSR | S_IWUSR);
    struct fdisk *disk = malloc(sizeof(struct fdisk));
    disk->fd = fd;
    return disk;
}

void fdisk_close(struct fdisk *disk)
{
    close(disk->fd);
}

void fdisk_read(struct fdisk *disk, uint8_t *buf, off_t offset, size_t count)
{
    lseek(disk->fd, offset, SEEK_SET);
    read(disk->fd, buf, count);
}

void fdisk_write(struct fdisk *disk, uint8_t *buf, off_t offset, size_t count)
{
    lseek(disk->fd, offset, SEEK_SET);
    write(disk->fd, buf, count);
}
