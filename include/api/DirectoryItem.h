#ifndef VFAT_DIRECTORYITEM_H
#define VFAT_DIRECTORYITEM_H

#include <iostream>
#include <string>

namespace org::vfat::api
{
    class DirectoryItem
    {
    public:
        virtual std::string GetName() const = 0;
        virtual tm* GetCreatedTime() const = 0;
        virtual tm* GetModifiedTime() const = 0;
        virtual ~DirectoryItem() { }
    };
}

#endif // VFAT_DIRECTORYITEM_H
