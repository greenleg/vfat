#include <assert.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "common.h"
#include "fdisk.h"

void fdisk_create(/*in*/ const char* fname, /*out*/ struct fdisk *disk)
{
    int fd = creat(fname, S_IRUSR | S_IWUSR);    
    disk->fd = fd;
    disk->closed = false;
}

void fdisk_open(const char* fname, struct fdisk *disk)
{
    int fd = open(fname, O_RDWR, S_IRUSR | S_IWUSR);
    disk->fd = fd;
    disk->closed = false;
}

void fdisk_close(struct fdisk *disk)
{
    close(disk->fd);
    disk->closed = true;
}

void fdisk_read(struct fdisk *disk, u8 *buf, off_t offset, size_t count)
{
    assert(!disk->closed);
    lseek(disk->fd, offset, SEEK_SET);
    read(disk->fd, buf, count);
}

void fdisk_write(struct fdisk *disk, u8 *buf, off_t offset, size_t count)
{
    assert(!disk->closed);
    lseek(disk->fd, offset, SEEK_SET);
    write(disk->fd, buf, count);
}
