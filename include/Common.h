#ifndef VFAT_COMMON_H
#define VFAT_COMMON_H

#include <stdint.h>
#include <time.h>
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

        static struct tm* GetTimeAndDate(uint32_t seconds)
        {
            time_t sec = (time_t)seconds;
            return localtime(&sec);
        }

        static std::string FormatDate(tm *time)
        {
            char text[100];
            strftime(text, sizeof(text) - 1, "%d/%m/%Y %H:%M", time);
            return std::string(text);
        }

        static std::string StringPadding(std::string original, size_t charCount)
        {
            original.resize(charCount, ' ');
            return original;
        }
    };
}

#endif /* VFAT_COMMON_H */
