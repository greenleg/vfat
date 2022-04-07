#ifndef VFAT_COMMANDLINE_H
#define VFAT_COMMANDLINE_H

#include <string>
#include <vector>
#include "../Common.h"

namespace org::vfat::cli
{
    class CommandLine
    {
    private:
        std::vector<std::string> args;

    public:
        CommandLine(int argc, char *argv[])
        {
            for (int i = 0; i < argc; i++) {
                args.push_back(argv[i]);
            }
        }

        CommandLine(const std::string& command)
        {
            Utils::StringSplit(command, this->args, ' ');
        }

        ~CommandLine()
        {
        }

        std::string GetArg(size_t index) const
        {
            return this->args.at(index);
        }

        size_t GetArgCount() const
        {
            return this->args.size();
        }

        bool HasOption(const std::string& flag)
        {
            auto iter = std::find(this->args.begin(), this->args.end(), flag);
            return iter != this->args.end();
        }

        std::string TryFetchByPrefix(const std::string& key)
        {
            for (auto iter = this->args.begin(); iter < this->args.end(); ++iter) {
                std::string& arg = *iter;
                if (arg.rfind(key, 0) == 0) {
                    return arg.substr(key.size());
                }
            }

            return "";
        }
    };
}

#endif // VFAT_COMMANDLINE_H
