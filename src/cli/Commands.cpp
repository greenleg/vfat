#include <iostream>
#include "../../include/cli/Commands.h"

using namespace std;
using namespace org::vfat::cli;

void Commands::Ls(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    Directory *currentDir = fsh->GetCurrentDirectory();

    vector<Directory *> directories;
    currentDir->GetDirectories(directories);
    vector<Directory *>::iterator dirIter;

    vector<File *> files;
    currentDir->GetFiles(files);
    vector<File *>::iterator fileIter;

    delete currentDir;

    if (cmdLine->HasOption("-all")) {
        for (dirIter = directories.begin(); dirIter < directories.end(); dirIter++) {
            Directory *subDir = *dirIter;
            string created = Utils::FormatDate(subDir->GetCreatedTime());

            cout << Utils::StringPadding(created, 20)
                 << Utils::StringPadding("<DIR>", 10)
                 << Utils::StringPadding("", 20)
                 << Utils::StringPadding(subDir->GetName(), 30)
                 << endl;

            delete subDir;
        }

        uint32_t totalFilesSize = 0;
        for (fileIter = files.begin(); fileIter < files.end(); fileIter++) {
            File *file = *fileIter;
            string created = Utils::FormatDate(file->GetCreatedTime());


            cout << Utils::StringPadding(created, 20)
                 << Utils::StringPadding("", 10)
                 << Utils::StringPadding(to_string(file->GetSize()) + " bytes", 20)
                 << Utils::StringPadding(file->GetName(), 30)
                 << endl;

            totalFilesSize += file->GetSize();

            delete file;
        }

        cout << Utils::StringPadding("", 10)
             << Utils::StringPadding(to_string(files.size()) + " File(s)", 20)
             << Utils::StringPadding(to_string(totalFilesSize) + " bytes", 20)
             << endl;

        uint32_t freeClusterCount = fsh->GetFileSystem()->GetFat()->GetFreeClusterCount();
        uint32_t bytesPerCluster = fsh->GetFileSystem()->GetBootSector()->GetBytesPerCluster();
        uint32_t freeSpaceInBytes = freeClusterCount * bytesPerCluster;

        cout << Utils::StringPadding("", 10)
             << Utils::StringPadding(to_string(directories.size()) + " Dir(s)", 20)
             << Utils::StringPadding(to_string(freeSpaceInBytes) + " bytes free", 20)
             << endl;
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
    Directory *currentDir = fsh->GetCurrentDirectory();
    currentDir->CreateDirectory(dirName);
    delete currentDir;
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
    Directory *currentDir = fsh->GetCurrentDirectory();
    currentDir->CreateFile(fileName);
    delete currentDir;
}

void Commands::Cat(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    if (cmdLine->GetArgCount() < 2) {
        throw std::logic_error("File is not specified.");
    }

    string fileName = cmdLine->GetArg(1);
    Directory *currentDir = fsh->GetCurrentDirectory();
    File *file = currentDir->GetFile(fileName);
    string text = file->ReadText(0, file->GetSize());
    delete file;
    delete currentDir;

    cout << text <<  endl;
}

void Commands::Import(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    if (cmdLine->GetArgCount() < 2) {
        throw std::logic_error("File or directory to import is not specified.");
    }

    string fileName = cmdLine->GetArg(1);
    Directory *currentDir = fsh->GetCurrentDirectory();
    currentDir->Import(fileName);
    delete currentDir;
}

void Commands::Mv(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    if (cmdLine->GetArgCount() < 3) {
        throw std::logic_error("At least one of the files is not specified.");
    }

    string srcFileName = cmdLine->GetArg(1);
    string destFileName = cmdLine->GetArg(2);
    Directory *currentDir = fsh->GetCurrentDirectory();
    currentDir->Move(srcFileName, destFileName);
    delete currentDir;
}

void Commands::Cp(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    if (cmdLine->GetArgCount() < 3) {
        throw std::logic_error("At least one of the files is not specified.");
    }

    string srcFileName = cmdLine->GetArg(1);
    string destFileName = cmdLine->GetArg(2);
    Directory *currentDir = fsh->GetCurrentDirectory();
    currentDir->Copy(srcFileName, destFileName);
    delete currentDir;
}

void Commands::Rm(CommandLine *cmdLine, FileSystemHandle *fsh)
{
    if (cmdLine->GetArgCount() < 2) {
        throw std::logic_error("File or directory is not specified.");
    }

    string fileName = cmdLine->GetArg(1);
    Directory *currentDir = fsh->GetCurrentDirectory();
    try {
        currentDir->DeleteFile(fileName);
    } catch (const std::ios_base::failure& error) {
        currentDir->DeleteDirectory(fileName);
    }

    delete currentDir;
}
