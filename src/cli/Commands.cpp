#include <iostream>
#include "../../include/cli/Commands.h"

using namespace std;
using namespace org::vfat::cli;

void Commands::Ls(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    vector<Directory *> directories;
    fsh->GetCurrentDirectory()->GetDirectories(directories);
    vector<Directory *>::iterator dirIter;

    vector<File *> files;
    fsh->GetCurrentDirectory()->GetFiles(files);
    vector<File *>::iterator fileIter;

    if (cmdLine->HasOption("-all")) {
        //string delim(30 + 20 + 12, '-');

        cout << Utils::StringPadding("NAME", 30)
             << Utils::StringPadding("CREATED", 20)
             << Utils::StringPadding("SIZE", 12)
             << endl;

        //cout << delim << endl;

        for (dirIter = directories.begin(); dirIter < directories.end(); dirIter++) {
            Directory *subDir = *dirIter;
            string created = Utils::FormatDate(subDir->GetCreatedTime());

            cout << Utils::StringPadding(subDir->GetName(), 30)
                 << Utils::StringPadding(created, 20)
                 << Utils::StringPadding("N/A", 12)
                 << endl;

            delete subDir;
        }

        for (fileIter = files.begin(); fileIter < files.end(); fileIter++) {
            File *file = *fileIter;
            string created = Utils::FormatDate(file->GetCreatedTime());

            cout << Utils::StringPadding(file->GetName(), 30)
                 << Utils::StringPadding(created, 20)
                 << Utils::StringPadding(to_string(file->GetSize()), 12)
                 << endl;

            delete file;
        }
    } else {
        // Simple short output
        for (dirIter = directories.begin(); dirIter < directories.end(); dirIter++) {
            Directory *subDir = *dirIter;
            cout << subDir->GetName() << " ";
            delete subDir;
        }

        for (fileIter = files.begin(); fileIter < files.end(); fileIter++) {
            File *file = *fileIter;
            cout << file->GetName() << " ";
            delete file;
        }

        if (directories.size() + files.size() > 0) {
            std::cout << endl;
        }
    }
}

void Commands::Mkdir(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    if (cmdLine->GetArgCount() < 2) {
        throw std::logic_error("Directory is not specified.");
    }

    string dirName = cmdLine->GetArg(1);
    fsh->GetCurrentDirectory()->CreateDirectory(dirName);
}

void Commands::Cd(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    string dirPath = cmdLine->GetArg(1);
    fsh->ChangeDirectory(dirPath);
}

void Commands::Touch(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    if (cmdLine->GetArgCount() < 2) {
        throw std::logic_error("File is not specified.");
    }

    string fileName = cmdLine->GetArg(1);
    fsh->GetCurrentDirectory()->CreateFile(fileName);
}

void Commands::Cat(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    if (cmdLine->GetArgCount() < 2) {
        throw std::logic_error("File is not specified.");
    }

    string fileName = cmdLine->GetArg(1);
    File *file = fsh->GetCurrentDirectory()->GetFile(fileName);
    string text = file->ReadText(0, file->GetSize());
    delete file;

    cout << text << endl;
}

void Commands::Import(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    if (cmdLine->GetArgCount() < 2) {
        throw std::logic_error("File is not specified.");
    }

    string fileName = cmdLine->GetArg(1);
    fsh->GetCurrentDirectory()->Import(fileName);
}

void Commands::Move(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    if (cmdLine->GetArgCount() < 3) {
        throw std::logic_error("At least one of the files is not specified.");
    }

    string srcFileName = cmdLine->GetArg(1);
    string destFileName = cmdLine->GetArg(2);
    fsh->GetCurrentDirectory()->Move(srcFileName, destFileName);
}
