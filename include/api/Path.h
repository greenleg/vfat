#ifndef VFAT_PATH_H
#define VFAT_PATH_H

#include <string>
#include <vector>

namespace org::vfat::api
{
    class Path
    {
    private:
        std::vector<std::string> items;

    public:
        Path() {} 
        Path(const Path& other);
        Path(Path&& other);
        Path& operator=(const Path& other);
        Path& operator=(Path&& other);

        void Combine(const std::string& path, bool normalize = false);
        std::string ToString(bool normalize = true) const;
        bool IsRoot() const;

        const std::string& GetItem(size_t index) const;
        const std::string& GetLastItem() const;
        size_t GetItemCount() const;
        Path GetParent() const;
    };
}

#endif // VFAT_PATH_H
