#ifndef VFAT_BINARYREADER_H
#define VFAT_BINARYREADER_H

#include <stdint.h>

namespace org::vfat
{
    class BinaryReader
    {
    public:
        static uint8_t  ReadUInt8(uint8_t *data, uint32_t offset);
        static uint16_t ReadUInt16(uint8_t *data, uint32_t offset);
        static uint32_t ReadUInt32(uint8_t *data, uint32_t offset);
        static uint64_t ReadUInt64(uint8_t *data, uint32_t offset);
        static void     WriteUInt8(uint8_t *data, uint32_t offset, uint8_t val);
        static void     WriteUInt16(uint8_t *data, uint32_t offset, uint16_t val);
        static void     WriteUInt32(uint8_t *data, uint32_t offset, uint32_t val);
        static void     WriteUInt64(uint8_t *data, uint32_t offset, uint64_t val);
    };
}

#endif /* VFAT_BINARYREADER_H */
