#ifndef VFAT_STREAM_EXT_DIRECTORY_ENTRY_H
#define VFAT_STREAM_EXT_DIRECTORY_ENTRY_H

#include <stdint.h>

struct stream_ext_dir_entry
{
    uint8_t entry_type;
    uint8_t secondary_flags;
    uint8_t name_length;
    uint32_t first_cluster;
    uint64_t data_length;
};

void stream_ext_dir_entry_readbuf(uint8_t *buf, struct stream_ext_dir_entry *entry);
void stream_ext_dir_entry_writebuf(struct stream_ext_dir_entry *entry, uint8_t *buf);

#endif /* VFAT_EXT_DIRECTORY_ENTRY_H */
