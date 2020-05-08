#include "../../include/api/Path.h"
#include "../../include/Common.h"

using namespace org::vfat::api;

Path::Path()
{
    this->items = new vector<string>();
}

Path::~Path()
{
    delete this->items;
}

void Path::Combine(std::string path, bool normalize)
{
    vector<string> items;
    Utils::StringSplit(path, items, '/');

    if (!path.empty() && path.at(0) == '/') {
        // This is an absolute path which starts from root;
        this->items->clear();
    }

    vector<string>::iterator iter;
    if (normalize) {
        for (iter = items.begin(); iter < items.end(); ++iter) {
            string name = *iter;
            if (name == ".") {
                continue;
            } else if (name == "..") {
                this->items->pop_back();
            } else {
                this->items->push_back(name);
            }
        }
    } else {
        for (iter = items.begin(); iter < items.end(); ++iter) {
            string name = *iter;
            this->items->push_back(name);
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
        string s = "";
        vector<string>::iterator iter;
        for (iter = this->items->begin(); iter < this->items->end(); ++iter) {
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

string Path::GetItem(size_t index) const
{
    return this->items->at(index);
}

size_t Path::GetItemCount() const
{
    return this->items->size();
}

Path* Path::Clone() const
{
    Path *other = new Path();
    vector<string>::iterator iter;
    for (iter = this->items->begin(); iter < this->items->end(); ++iter) {
        other->items->push_back(*iter);
    }

    return other;
}

Path* Path::GetParent() const
{
    Path *other = new Path();
    vector<string>::iterator iter;
    for (iter = this->items->begin(); iter < this->items->end() - 1; ++iter) {
        other->items->push_back(*iter);
    }

    return other;
}
