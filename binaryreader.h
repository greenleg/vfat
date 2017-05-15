#ifndef VFAT_BINARYREADER_H
#define VFAT_BINARYREADER_H

#include <stdint.h>

uint8_t  read_u8(uint8_t *data, uint32_t offset);
uint32_t read_u32(uint8_t *data, uint32_t offset);
uint64_t read_u64(uint8_t *data, uint32_t offset);
void     write_u8(uint8_t *data, uint32_t offset, uint8_t val);
void     write_u32(uint8_t *data, uint32_t offset, uint32_t val);
void     write_u64(uint8_t *data, uint32_t offset, uint64_t val);

#endif /* VFAT_BINARYREADER_H */
