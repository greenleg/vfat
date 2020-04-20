#include <iostream>
#include <cstring>
#include <exception>
#include "../include/api/FileSystem.h"
#include "../include/api/Directory.h"
#include "../include/api/File.h"
#include "../include/Common.h"
#include "../include/cli/FileSystemHandle.h"

using namespace std;
using namespace org::vfat::api;
using namespace org::vfat::cli;

class Command
{
private:
    vector<string> args;

public:
    Command(int argc, char *argv[])
    {
        //this->args = new vector<string>();
        for (int i = 0; i < argc; i++) {
            args.push_back(argv[i]);
        }
    }

    Command(string command)
    {
        //this->args = new vector<string>();
        Utils::StringSplit(command, this->args, ' ');
    }

    ~Command()
    {
        //delete this->args;
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

    string FetchByPrefix(string key)
    {
        vector<string>::iterator iter;
        for (iter = this->args.begin(); iter < this->args.end(); iter++) {
            string arg = *iter;
            if (arg.rfind(key, 0) == 0) {
                return arg.substr(key.size());
            }
        }

        throw new std::logic_error("Parameter not found.");
    }
};

void ls(Command *cmd, FileSystemHandle *fsh)
{
    vector<Directory *> directories;
    fsh->GetCurrentDirectory()->GetDirectories(directories);
    vector<Directory *>::iterator iter;
    for (iter = directories.begin(); iter < directories.end(); iter++) {
        Directory *subDir = *iter;
        printf("%s ", subDir->GetName().c_str());
        delete subDir;
    }

    if (directories.size() > 0) {
        std::cout << endl;
    }
}

void mkdir(Command *cmd, FileSystemHandle *fsh)
{
    try {
        string dirName = cmd->GetArg(1);
        fsh->GetCurrentDirectory()->CreateDirectory(dirName);
    } catch (exception err) {
        // Save all changes.
        fsh->GetFileSystem()->Write();
    }
}

void cd(Command *cmd, FileSystemHandle *fsh)
{
    string dirPath = cmd->GetArg(1);
    fsh->ChangeDirectory(dirPath);
}

int ProcessCommand(string input, FileSystemHandle *fsh)
{
    Command cmd(input);
    string cmdName = cmd.GetArg(0);

    if (cmdName == "exit") {
        fsh->GetFileSystem()->Write();
        return 1;
    }

    if (cmdName == "ls") {
        ls(&cmd, fsh);
        return 0;
    }

    if (cmdName == "mkdir") {
        mkdir(&cmd, fsh);
        return 0;
    }

    if (cmdName == "cd") {
        cd(&cmd, fsh);
        return 0;
    }

    printf("Unknown command: %s\r\n", input.c_str());
    return 0;
}

int main(int argc, char *argv[])
{
//    for (int i = 0; i < argc; i++) {
//        cout << "Arg " << i << ": " << argv[i] << endl;
//    }

    Command cmd(argc, argv);
    string devName = cmd.FetchByPrefix("-dev:");

    FileSystemHandle fsh(devName);
    if (cmd.HasOption("-f")) {
        fsh.Format(1024 * 1024, 512, 1);
    } else {
        fsh.Read();
    }

    string input;
    std::getline(std::cin, input);

    while (true) {
        int res = ProcessCommand(input, &fsh);
        if (res != 0) {
            break;
        }

        std::getline(std::cin, input);
    }

    return 0;
}


