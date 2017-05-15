#ifndef VFAT_SECTOR_H
#define VFAT_SECTOR_H

#include <stdint.h>

struct sector
{
    uint8_t *data;
    uint16_t size;
};

struct sector *sector_create(uint16_t sectorSize);

uint8_t  sector_read_uint8(struct sector *sector, uint16_t offset);
uint32_t sector_read_uint32(struct sector *sector, uint16_t offset);
void     sector_write_uint8(struct sector *sector, uint16_t offset, uint8_t val);
void     sector_write_uint32(struct sector *sector, uint16_t offset, uint32_t val);

#endif
