#ifndef VFAT_FILE_DIRECTORY_ENTRY_H
#define VFAT_FILE_DIRECTORY_ENTRY_H

#include <stdint.h>

#include "fdisk.h"

#define FILE_DIR_ENTRY_TYPE 0x85

struct file_dir_entry
{
    uint8_t entry_type;
    uint8_t secondary_count;
    uint16_t attributes;
    uint32_t create;
    uint32_t last_modified;
    uint32_t last_accessed;
};

void file_dir_entry_readbuf(uint8_t *buf, struct file_dir_entry *entry);
void file_dir_entry_writebuf(struct file_dir_entry *entry, uint8_t * buf);

void file_dir_entry_read(struct fdisk* disk, struct file_dir_entry *entry);
void file_dir_entry_write(struct file_dir_entry *entry, struct fdisk* disk);

#endif /* VFAT_FILE_DIRECTORY_ENTRY_H */
