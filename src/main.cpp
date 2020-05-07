#include <iostream>
#include <cstring>
#include <exception>
#include "../include/api/FileSystem.h"
#include "../include/Common.h"
#include "../include/cli/CommandLine.h"
#include "../include/cli/FileSystemHandle.h"
#include "../include/cli/Commands.h"

using namespace std;
using namespace org::vfat::api;
using namespace org::vfat::cli;

int ProcessCommand(string input, FileSystemHandle *fsh);

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

//    ProcessCommand("tree", &fsh);
//    fsh.GetFileSystem()->Write();


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

        if (cmdName == "ls") {
            Commands::Ls(&cmdLine, fsh);
            return 0;
        }

        if (cmdName == "mkdir") {
            Commands::Mkdir(&cmdLine, fsh);
            return 0;
        }

        if (cmdName == "cd") {
            Commands::Cd(&cmdLine, fsh);
            return 0;
        }

        if (cmdName == "touch") {
            Commands::Touch(&cmdLine, fsh);
            return 0;
        }

        if (cmdName == "cat") {
            Commands::Cat(&cmdLine, fsh);
            return 0;
        }

        if (cmdName == "import") {
            Commands::Import(&cmdLine, fsh);
            return 0;
        }

        if (cmdName == "mv") {
            Commands::Mv(&cmdLine, fsh);
            return 0;
        }

        if (cmdName == "cp") {
            Commands::Cp(&cmdLine, fsh);
            return 0;
        }

        if (cmdName == "rm") {
            Commands::Rm(&cmdLine, fsh);
            return 0;
        }

        if (cmdName == "tree") {
            Commands::Tree(&cmdLine, fsh);
            return 0;
        }

        printf("Unknown command: %s\r\n", input.c_str());
        return 0;
    } catch (const exception& err) {
        cout << err.what() << endl;
        return 0;
    }
}
