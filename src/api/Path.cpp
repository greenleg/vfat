#include "../../include/api/Path.h"
#include "../../include/Common.h"

using namespace org::vfat::api;

Path::Path(/*std::string path*/)
{
    this->items = new vector<string>();

//    vector<string> items;
//    Utils::StringSplit(path, items, '/');

//    vector<string>::iterator iter;
//    for (iter = items.begin(); iter < items.end(); ++iter) {
//        string name = *iter;
//        if (name == ".") {
//            continue;
//        } else if (name == "..") {
//            this->items->pop_back();
//        } else {
//            this->items->push_back(name);
//        }
//    }
}

Path::~Path()
{
    delete this->items;
}

void Path::Combine(std::string path, bool normalize)
{
    vector<string> items;
    Utils::StringSplit(path, items, '/');

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

std::string Path::ToString() const
{
    if (this->IsRoot()) {
        return "/";
    }

    string s = "";
    vector<string>::iterator iter;
    for (iter = this->items->begin(); iter < this->items->end(); ++iter) {
        s.append("/");
        s.append(*iter);
    }

    return s;
}

bool Path::IsRoot() const
{
    return (GetItemCount() == 0);
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
