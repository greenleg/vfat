#include <iostream>
#include <cstring>
#include <exception>
#include <map>
#include "../include/api/FileSystem.h"
#include "../include/Common.h"
#include "../include/cli/CommandLine.h"
#include "../include/cli/FileSystemHelper.h"
#include "../include/cli/Commands.h"

using namespace std;
using namespace org::vfat::api;
using namespace org::vfat::cli;

int ProcessCommand(string input, FileSystemHelper *fsh);

/**
 * Points to a function that implements a command.
 */
typedef void (*CmdImplFunc)(CommandLine *cmdLine, FileSystemHelper *fsh);

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
    string devName = cmdLine.TryFetchByPrefix("-dev:");
    if (devName == "") {
        cout << "Device is not specified." << endl;
        return 1;
    }

    FileSystemHelper fsh(devName);
    if (cmdLine.HasOption("-f")) {
        uint64_t volumeSize;
        string volumeSizeStr = cmdLine.TryFetchByPrefix("-size:");
        if (volumeSizeStr != "") {
            volumeSize = std::stoul(volumeSizeStr) * 1024 * 1024;
        } else {
            // The volume size is 10MB by default;
            volumeSize = 10 * 1024 * 1024;
            cout << "The '-size' option is not specified, the default value 10MB will be used." << endl;
        }

        fsh.Format(volumeSize, 512, 1);
    } else {
        fsh.Read();
    }

    // Print the command line prompt;
    string fullPath = fsh.GetCurrentPath().ToString(true);
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
        string fullPath = fsh.GetCurrentPath().ToString(true);
        cout << fullPath << "$ ";

        std::getline(std::cin, input);
    }

    return 0;
}

int ProcessCommand(string input, FileSystemHelper *fsh)
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
