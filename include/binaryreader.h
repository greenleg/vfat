#ifndef VFAT_BINARYREADER_H
#define VFAT_BINARYREADER_H

#include <stdint.h>
#include "common.h"

u8  read_u8(u8 *data, u32 offset);
u16 read_u16(u8 *data, u32 offset);
u32 read_u32(u8 *data, u32 offset);
u64 read_u64(u8 *data, u32 offset);
void     write_u8(u8 *data, u32 offset, u8 val);
void     write_u16(u8 *data, u32 offset, u16 val);
void     write_u32(u8 *data, u32 offset, u32 val);
void     write_u64(u8 *data, u32 offset, u64 val);

#endif /* VFAT_BINARYREADER_H */
