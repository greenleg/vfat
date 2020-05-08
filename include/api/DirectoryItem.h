#ifndef VFAT_DIRECTORYITEM_H
#define VFAT_DIRECTORYITEM_H

#include <iostream>
#include <string>

namespace org::vfat::api
{
using namespace std;
    class DirectoryItem
    {
    public:
        virtual string GetName() const = 0;
        virtual tm* GetCreatedTime() const = 0;
        virtual tm* GetModifiedTime() const = 0;
        virtual ~DirectoryItem() { }
    };
}

#endif // VFAT_DIRECTORYITEM_H
