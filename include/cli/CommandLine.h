#ifndef VFAT_COMMANDLINE_H
#define VFAT_COMMANDLINE_H

#include <string>
#include <vector>
#include "../Common.h"

using namespace std;

namespace org::vfat::cli
{
    class CommandLine
    {
    private:
        vector<string> args;

    public:
        CommandLine(int argc, char *argv[])
        {
            for (int i = 0; i < argc; i++) {
                args.push_back(argv[i]);
            }
        }

        CommandLine(string command)
        {
            Utils::StringSplit(command, this->args, ' ');
        }

        ~CommandLine()
        {
        }

        string GetArg(size_t index) const
        {
            return this->args.at(index);
        }

        size_t GetArgCount() const
        {
            return this->args.size();
        }

        bool HasOption(string flag)
        {
            vector<string>::iterator iter = std::find(this->args.begin(), this->args.end(), flag);
            return iter != this->args.end();
        }

//        string FetchByPrefix(string key)
//        {
//            vector<string>::iterator iter;
//            for (iter = this->args.begin(); iter < this->args.end(); iter++) {
//                string arg = *iter;
//                if (arg.rfind(key, 0) == 0) {
//                    return arg.substr(key.size());
//                }
//            }

//            throw new std::logic_error("Parameter not found.");
//        }

        string TryFetchByPrefix(string key)
        {
            vector<string>::iterator iter;
            for (iter = this->args.begin(); iter < this->args.end(); iter++) {
                string arg = *iter;
                if (arg.rfind(key, 0) == 0) {
                    return arg.substr(key.size());
                }
            }

            return "";
        }
    };
}

#endif // VFAT_COMMANDLINE_H
