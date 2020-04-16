#ifndef VFAT_PATH_H
#define VFAT_PATH_H

#include <string>
#include <vector>

//using namespace org::vfat;
using namespace std;

namespace org::vfat::api
{
    class Path
    {
    private:
        //static Path *root = nullptr;// = new Path("/");
        vector<string> *items;

    public:
        //static Path* GetRoot() { return new Path(); }

        Path();
        ~Path();
        //Path* Combine(std::string path) const;
        void Combine(std::string path, bool normalize = false);
        Path* Clone() const;
        std::string ToString() const;
        bool IsRoot() const;

        string GetItem(size_t index) const;
        size_t GetItemCount() const;
    };
}

#endif // VFAT_PATH_H
