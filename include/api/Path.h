#ifndef VFAT_PATH_H
#define VFAT_PATH_H

#include <string>

//using namespace org::vfat;

namespace org::vfat::api
{
    class Path
    {
    private:
        //static Path *root = nullptr;// = new Path("/");

    public:
        static Path* GetRoot() { return new Path("/"); }

        Path(std::string path);
        Path* Combine(std::string path);
        std::string ToString() const;
        bool IsRoot() const;

        std::string GetName(size_t index) const;
        size_t GetSize() const;
    };
}

#endif // VFAT_PATH_H
