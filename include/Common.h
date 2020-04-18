#ifndef VFAT_COMMON_H
#define VFAT_COMMON_H

#include <stdint.h>
#include <string>
#include <cstring>
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
                if (!token.empty()) {
                    container.push_back(token);
                }
            }
        }

//        static uint8_t* StringToBytes(std::string s)
//        {
//            const char *ascii = s.c_str();
//            uint8_t *buffer = new uint8_t[s.size()];
//            memcpy(buffer, ascii, s.size());

//            return buffer;
//        }
    };
}

#endif /* VFAT_COMMON_H */
