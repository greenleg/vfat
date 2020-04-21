#include <iostream>
#include "../../include/cli/Commands.h"

using namespace std;
using namespace org::vfat::cli;

void Commands::ls(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    vector<Directory *> directories;
    fsh->GetCurrentDirectory()->GetDirectories(directories);
    vector<Directory *>::iterator dirIter;

    vector<File *> files;
    fsh->GetCurrentDirectory()->GetFiles(files);
    vector<File *>::iterator fileIter;

    if (cmdLine->HasOption("-all")) {
        string delim(30 + 20 + 12, '-');

        cout << Utils::StringPadding("Name", 30)
             << Utils::StringPadding("Created", 20)
             << Utils::StringPadding("Size", 12)
             << endl;

        cout << delim << endl;

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

        if (directories.size() > 0) {
            std::cout << endl;
        }
    }
}

void Commands::mkdir(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    if (cmdLine->GetArgCount() < 2) {
        throw std::logic_error("Directory name is not specified.");
    }

    string dirName = cmdLine->GetArg(1);
    fsh->GetCurrentDirectory()->CreateDirectory(dirName);
}

void Commands::cd(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    string dirPath = cmdLine->GetArg(1);
    fsh->ChangeDirectory(dirPath);
}
