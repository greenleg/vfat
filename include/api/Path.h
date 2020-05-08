#ifndef VFAT_PATH_H
#define VFAT_PATH_H

#include <string>
#include <vector>

using namespace std;

namespace org::vfat::api
{
    class Path
    {
    private:
        vector<string> *items;

    public:
        Path();
        ~Path();
        void Combine(std::string path, bool normalize = false);
        Path* Clone() const;
        std::string ToString(bool normalize = true) const;
        bool IsRoot() const;

        string GetItem(size_t index) const;
        size_t GetItemCount() const;
        Path* GetParent() const;
    };
}

#endif // VFAT_PATH_H
