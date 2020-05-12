#ifndef VFAT_DEVICE_H
#define VFAT_DEVICE_H

#include <stdint.h>
#include <sys/types.h>

namespace org::vfat
{
    /**
     * @brief The abstract class representing a primary storage (a file on a disk or a memory block);
     */
    class Device
    {
    public:
        virtual void Create() = 0;
        virtual void Open() = 0;
        virtual void Close() = 0;
        virtual void Delete() = 0;
        virtual void Read(uint8_t *buffer, long int fileOffset, size_t count) const = 0;
        virtual void Write(uint8_t *buffer, long int fileOffset, size_t count) const = 0;

        virtual ~Device() { };
    };
}

#endif // VFAT_DEVICE_H
