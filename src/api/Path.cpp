#include "../../include/api/Path.h"
#include "../../include/Common.h"

using namespace org::vfat::api;



Path::Path(const Path& other) :
    items(other.items) 
{ }

Path::Path(Path&& other) : 
    items(std::move(other.items)) 
{ }

Path& Path::operator=(const Path& other)
{
    if (this != &other) {
        items = other.items;
    }

    return *this;
}

Path& Path::operator=(Path&& other)
{
    if (this != &other) {
        items = std::move(other.items);
    }

    return *this;
}

void Path::Combine(const std::string& path, bool normalize)
{
    if (!path.empty() && path.at(0) == '/') {
        // This is an absolute path which starts from root;
        this->items.clear();
    }
    
    std::vector<std::string> items;
    Utils::StringSplit(path, items, '/');

    if (normalize) {
        for (auto iter = items.begin(); iter < items.end(); ++iter) {
            std::string& name = *iter;
            if (name == ".") {
                continue;
            } else if (name == "..") {
                this->items.pop_back();
            } else {
                this->items.push_back(name);
            }
        }
    } else {
        for (auto iter = items.begin(); iter < items.end(); ++iter) {
            std::string& name = *iter;
            this->items.push_back(name);
        }
    }
}

std::string Path::ToString(bool normalize) const
{
    if (this->IsRoot()) {
        return "/";
    }

    if (normalize) {
        Path normalizedPath;
        normalizedPath.Combine(this->ToString(false), true);
        return normalizedPath.ToString(false);
    } else {
        // Print as is;
        std::string s = "";
        for (auto iter = this->items.begin(); iter < this->items.end(); ++iter) {
            s.append("/");
            s.append(*iter);
        }

        return s;
    }
}

bool Path::IsRoot() const
{
    return (this->GetItemCount() == 0);

//    int numberOfItems = 0;
//    vector<string>::iterator iter;
//    for (iter = this->items->begin(); iter < this->items->end(); ++iter) {
//        string name = *iter;
//        if (name == ".") {
//            continue;
//        } else if (name == "..") {
//            --numberOfItems;
//        } else {
//            ++numberOfItems;
//        }
//    }

//    return numberOfItems == 0;
}

std::string Path::GetItem(size_t index) const
{
    return this->items[index];
}

size_t Path::GetItemCount() const
{
    return this->items.size();
}

Path Path::GetParent() const
{
    Path result;
    for (auto iter = items.begin(); iter < items.end() - 1; ++iter) {
        result.items.push_back(*iter);
    }

    return result;
}
