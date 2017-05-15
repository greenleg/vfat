#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>

#include "sector.h"

struct sector *sector_create(uint16_t sectorSize)
{
    struct sector *sector = malloc(sizeof(struct sector));
    sector->data = malloc(sectorSize);
    sector->size = sectorSize;
    return sector;
}

uint8_t sector_read_uint8(struct sector *sector, uint16_t offset)
{
    return sector->data[offset];
}

uint32_t sector_read_uint32(struct sector *sector, uint16_t offset)
{
    uint8_t b0 = sector->data[offset + 0];
    uint8_t b1 = sector->data[offset + 1];
    uint8_t b2 = sector->data[offset + 2];
    uint8_t b3 = sector->data[offset + 3];
    return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}

void sector_write_uint8(struct sector *sector, uint16_t offset, uint8_t val)
{
    sector->data[offset] = val;
}

void sector_write_uint32(struct sector *sector, uint16_t offset, uint32_t val)
{
    sector->data[offset + 0] = val & 0xFF;
    sector->data[offset + 1] = (val >> 8) & 0xFF;
    sector->data[offset + 2] = (val >> 16) & 0xFF;
    sector->data[offset + 3] = (val >> 24) & 0xFF;
}
