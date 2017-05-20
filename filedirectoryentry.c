#include "binaryreader.h"
#include "filedirectoryentry.h"

#define FDE_ENTRYTYPE_OFFSET 0
#define FDE_SECONDARYCOUNT_OFFSET 1
#define FDE_ATTRIBUTES_OFFSET 4
#define FDE_CREATE_OFFSET 8
#define FDE_LASTMODIFIED_OFFSET 12
#define FDE_LASTACCESSED_OFFSET 16

void file_dir_entry_readbuf(uint8_t *buf, struct file_dir_entry *entry)
{
    entry->entry_type = read_u8(buf, FDE_ENTRYTYPE_OFFSET);
    entry->secondary_count = read_u8(buf, FDE_SECONDARYCOUNT_OFFSET);
    entry->attributes = read_u16(buf, FDE_ATTRIBUTES_OFFSET);
    entry->create = read_u32(buf, FDE_CREATE_OFFSET);
    entry->last_modified = read_u32(buf, FDE_LASTMODIFIED_OFFSET);
    entry->last_accessed = read_u32(buf, FDE_LASTACCESSED_OFFSET);
}

void file_dir_entry_writebuf(struct file_dir_entry *entry, uint8_t * buf)
{
    write_u8(buf, FDE_ENTRYTYPE_OFFSET, entry->entry_type);
    write_u8(buf, FDE_SECONDARYCOUNT_OFFSET, entry->secondary_count);
    write_u16(buf, FDE_ATTRIBUTES_OFFSET, entry->attributes);
    write_u32(buf, FDE_CREATE_OFFSET, entry->create);
    write_u32(buf, FDE_LASTMODIFIED_OFFSET, entry->last_modified);
    write_u32(buf, FDE_LASTACCESSED_OFFSET, entry->last_accessed);
}

void file_dir_entry_read(struct fdisk* disk, struct file_dir_entry *entry)
{

}

void file_dir_entry_write(struct file_dir_entry *entry, struct fdisk* disk)
{

}
