#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>

#include "binaryreader.h"

uint8_t read_u8(uint8_t *data, uint32_t offset)
{
    return data[offset];
}

uint16_t read_u16(uint8_t *data, uint32_t offset)
{
    uint32_t b0 = data[offset + 0];
    uint32_t b1 = data[offset + 1];
    return (b1 << 8) | b0;
}

uint32_t read_u32(uint8_t *data, uint32_t offset)
{
    uint32_t b0 = data[offset + 0];
    uint32_t b1 = data[offset + 1];
    uint32_t b2 = data[offset + 2];
    uint32_t b3 = data[offset + 3];
    return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}

uint64_t read_u64(uint8_t *data, uint32_t offset)
{
    uint64_t b0 = data[offset + 0];
    uint64_t b1 = data[offset + 1];
    uint64_t b2 = data[offset + 2];
    uint64_t b3 = data[offset + 3];
    uint64_t b4 = data[offset + 4];
    uint64_t b5 = data[offset + 5];
    uint64_t b6 = data[offset + 6];
    uint64_t b7 = data[offset + 7];
    return (b7 << 56) | (b6 << 48) | (b5 << 40) | (b4 << 32) | (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}

void write_u8(uint8_t *data, uint32_t offset, uint8_t val)
{
    data[offset] = val;
}

void write_u16(uint8_t *data, uint32_t offset, uint16_t val)
{
    data[offset + 0] = val & 0xFF;
    data[offset + 1] = (val >> 8) & 0xFF;
}

void write_u32(uint8_t *data, uint32_t offset, uint32_t val)
{
    data[offset + 0] = val & 0xFF;
    data[offset + 1] = (val >> 8) & 0xFF;
    data[offset + 2] = (val >> 16) & 0xFF;
    data[offset + 3] = (val >> 24) & 0xFF;
}

void write_u64(uint8_t *data, uint32_t offset, uint64_t val)
{
    data[offset + 0] = val & 0xFF;
    data[offset + 1] = (val >> 8) & 0xFF;
    data[offset + 2] = (val >> 16) & 0xFF;
    data[offset + 3] = (val >> 24) & 0xFF;
    data[offset + 4] = (val >> 32) & 0xFF;
    data[offset + 5] = (val >> 40) & 0xFF;
    data[offset + 6] = (val >> 48) & 0xFF;
    data[offset + 7] = (val >> 56) & 0xFF;
}
