#include "streamextdirectoryentry.h"
#include "binaryreader.h"

#define SEDE_ENTRYTYPE_OFFSET 0
#define SEDE_SECONDARYFLAGS_OFFSET 1
#define SEDE_NAMELENGTH_OFFSET 3
#define SEDE_FIRSTCLUSTER_OFFSET 20
#define SEDE_DATALENGTH_OFFSET 24

void stream_ext_dir_entry_readbuf(uint8_t *buf, struct stream_ext_dir_entry *entry)
{
    entry->entry_type = read_u8(buf, SEDE_ENTRYTYPE_OFFSET);
    entry->secondary_flags = read_u8(buf, SEDE_SECONDARYFLAGS_OFFSET);
    entry->name_length = read_u8(buf, SEDE_NAMELENGTH_OFFSET);
    entry->first_cluster = read_u32(buf, SEDE_FIRSTCLUSTER_OFFSET);
    entry->data_length = read_u64(buf, SEDE_DATALENGTH_OFFSET);
}

void stream_ext_dir_entry_writebuf(struct stream_ext_dir_entry *entry, uint8_t *buf)
{
    write_u8(buf, SEDE_ENTRYTYPE_OFFSET, entry->entry_type);
    write_u8(buf, SEDE_SECONDARYFLAGS_OFFSET, entry->secondary_flags);
    write_u8(buf, SEDE_NAMELENGTH_OFFSET, entry->name_length);
    write_u32(buf, SEDE_FIRSTCLUSTER_OFFSET, entry->first_cluster);
    write_u64(buf, SEDE_DATALENGTH_OFFSET, entry->data_length);
}
