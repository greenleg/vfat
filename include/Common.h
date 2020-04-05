#ifndef VFAT_COMMON_H
#define VFAT_COMMON_H

#include <stdint.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

namespace org::vfat
{
    class Utils
    {
    public:
        static void StringSplit(const std::string& str, std::vector<std::string>& container, char delim = ' ')
        {
            std::stringstream ss(str);
            std::string token;
            while (std::getline(ss, token, delim)) {
                container.push_back(token);
            }
        }
    };
}

#endif /* VFAT_COMMON_H */
