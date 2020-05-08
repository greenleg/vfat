#include <iostream>
#include <cstring>
#include <exception>
#include <map>
#include "../include/api/FileSystem.h"
#include "../include/Common.h"
#include "../include/cli/CommandLine.h"
#include "../include/cli/FileSystemHandle.h"
#include "../include/cli/Commands.h"

using namespace std;
using namespace org::vfat::api;
using namespace org::vfat::cli;

int ProcessCommand(string input, FileSystemHandle *fsh);

typedef void (*CmdImplFunc)(CommandLine *cmdLine, FileSystemHandle *fsh);

std::map<string, CmdImplFunc> CmdImplMap
{
    { "ls", Commands::Ls },
    { "mkdir", Commands::Mkdir },
    { "cd", Commands::Cd },
    { "touch", Commands::Touch },
    { "cat", Commands::Cat },
    { "import", Commands::Import },
    { "mv", Commands::Mv },
    { "cp", Commands::Cp },
    { "rm", Commands::Rm },
    { "tree", Commands::Tree },
};


int main(int argc, char *argv[])
{
    CommandLine cmdLine(argc, argv);
    string devName = cmdLine.FetchByPrefix("-dev:");

    FileSystemHandle fsh(devName);
    if (cmdLine.HasOption("-f")) {
        string volumeSizeStr = cmdLine.FetchByPrefix("-size:");
        uint64_t volumeSize = std::stoul(volumeSizeStr) * 1024 * 1024;
        fsh.Format(volumeSize, 512, 1);
    } else {
        fsh.Read();
    }

    // Print the command line prompt;
    string fullPath = fsh.GetCurrentPath()->ToString(true);
    cout << fullPath << "$ ";

    string input;
    std::getline(std::cin, input);

    while (true) {
        int res = ProcessCommand(input, &fsh);
        fsh.GetFileSystem()->Write();
        if (res != 0) {
            break;
        }

        // Print the command line prompt;
        string fullPath = fsh.GetCurrentPath()->ToString(true);
        cout << fullPath << "$ ";

        std::getline(std::cin, input);
    }

    return 0;
}

int ProcessCommand(string input, FileSystemHandle *fsh)
{
    try {
        CommandLine cmdLine(input);
        if (cmdLine.GetArgCount() == 0) {
            // Empty command, nothing to do.
            return 0;
        }

        string cmdName = cmdLine.GetArg(0);

        if (cmdName == "exit") {
            fsh->GetFileSystem()->Write();
            return 1;
        }

        auto iter = CmdImplMap.find(cmdName);
        if (iter == CmdImplMap.end()) {
            printf("Unknown command: %s\r\n", input.c_str());
        } else {
            (*(iter->second))(&cmdLine, fsh);
        }

        return 0;
    } catch (const exception& err) {
        cout << err.what() << endl;
        return 0;
    }
}
