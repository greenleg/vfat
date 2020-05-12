#ifndef VFAT_RAMDISK_H
#define VFAT_RAMDISK_H

#include <stdexcept>
#include "Device.h"

namespace org::vfat
{
using namespace std;

    class RamDisk : public Device
    {

    public:
        ~RamDisk() {}
        void Create() { throw runtime_error("The operation is not implemented."); }
        void Open() { throw runtime_error("The operation is not implemented."); }
        void Close() { throw runtime_error("The operation is not implemented."); }
        void Delete() { throw runtime_error("The operation is not implemented."); }
        void Read(uint8_t *buffer, long int fileOffset, size_t count) const { throw runtime_error("The operation is not implemented."); }
        void Write(uint8_t *buffer, long int fileOffset, size_t count) const { throw runtime_error("The operation is not implemented."); }
    };
}

#endif // VFAT_RAMDISK_H
