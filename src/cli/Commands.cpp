#include <iostream>
#include "../../include/cli/Commands.h"

using namespace org::vfat::cli;

void Commands::Ls(CommandLine *cmdLine, FileSystemHelper *fsh)
{
    Directory currentDir = fsh->GetCurrentDirectory();

    std::vector<Directory> directories = currentDir.GetDirectories();
    std::vector<Directory>::iterator dirIter;

    std::vector<File> files = currentDir.GetFiles();
    std::vector<File>::iterator fileIter;

    if (cmdLine->HasOption("-all")) {
        for (dirIter = directories.begin(); dirIter < directories.end(); ++dirIter) {
            Directory subDir = *dirIter;
            std::string created = Utils::FormatDate(subDir.GetCreatedTime());

            cout << Utils::StringPadding(created, 20)
                 << Utils::StringPadding("<DIR>", 10)
                 << Utils::StringPadding("", 20)
                 << Utils::StringPadding(subDir.GetName(), 30)
                 << endl;
        }

        uint32_t totalFilesSize = 0;
        for (fileIter = files.begin(); fileIter < files.end(); ++fileIter) {
            File file = *fileIter;
            string created = Utils::FormatDate(file.GetCreatedTime());

            cout << Utils::StringPadding(created, 20)
                 << Utils::StringPadding("", 10)
                 << Utils::StringPadding(to_string(file.GetSize()) + " bytes", 20)
                 << Utils::StringPadding(file.GetName(), 30)
                 << endl;

            totalFilesSize += file.GetSize();
        }

        cout << Utils::StringPadding("", 10)
             << Utils::StringPadding(to_string(files.size()) + " File(s)", 20)
             << Utils::StringPadding(to_string(totalFilesSize) + " bytes", 20)
             << endl;

        uint32_t freeClusterCount = fsh->GetFileSystem()->GetFat().GetFreeClusterCount();
        uint32_t bytesPerCluster = fsh->GetFileSystem()->GetBootSector().GetBytesPerCluster();
        uint32_t freeSpaceInBytes = freeClusterCount * bytesPerCluster;

        cout << Utils::StringPadding("", 10)
             << Utils::StringPadding(to_string(directories.size()) + " Dir(s)", 20)
             << Utils::StringPadding(to_string(freeSpaceInBytes) + " bytes free", 20)
             << endl;
    } else {
        // Simple short output
        for (dirIter = directories.begin(); dirIter < directories.end(); ++dirIter) {
            Directory subDir = *dirIter;
            cout << subDir.GetName() << " ";
        }

        for (fileIter = files.begin(); fileIter < files.end(); ++fileIter) {
            File file = *fileIter;
            cout << file.GetName() << " ";
        }

        if (directories.size() + files.size() > 0) {
            std::cout << endl;
        }
    }
}

void Commands::Mkdir(CommandLine *cmdLine, FileSystemHelper *fsh)
{
    if (cmdLine->GetArgCount() < 2) {
        throw std::logic_error("Directory is not specified.");
    }

    std::string dirName = cmdLine->GetArg(1);
    Directory currentDir = fsh->GetCurrentDirectory();
    currentDir.CreateDirectory(dirName);
}

void Commands::Cd(CommandLine *cmdLine, FileSystemHelper *fsh)
{
    std::string dirPath = cmdLine->GetArg(1);
    fsh->ChangeDirectory(dirPath);
}

void Commands::Touch(CommandLine *cmdLine, FileSystemHelper *fsh)
{
    if (cmdLine->GetArgCount() < 2) {
        throw std::logic_error("File is not specified.");
    }

    std::string fileName = cmdLine->GetArg(1);
    Directory currentDir = fsh->GetCurrentDirectory();
    currentDir.CreateFile(fileName);
}

void Commands::Cat(CommandLine *cmdLine, FileSystemHelper *fsh)
{
    if (cmdLine->GetArgCount() < 2) {
        throw std::logic_error("File is not specified.");
    }

    std::string fileName = cmdLine->GetArg(1);
    Directory currentDir = fsh->GetCurrentDirectory();
    File file = currentDir.GetFile(fileName);
    std::string text = file.ReadText(0, file.GetSize());

    std::cout << text << std::endl;
}

void Commands::Import(CommandLine *cmdLine, FileSystemHelper *fsh)
{
    if (cmdLine->GetArgCount() < 2) {
        throw std::logic_error("File or directory to import is not specified.");
    }

    std::string fileName = cmdLine->GetArg(1);
    Directory currentDir = fsh->GetCurrentDirectory();
    currentDir.Import(fileName);
}

void Commands::Mv(CommandLine *cmdLine, FileSystemHelper *fsh)
{
    if (cmdLine->GetArgCount() < 3) {
        throw std::logic_error("At least one of the files is not specified.");
    }

    std::string srcFileName = cmdLine->GetArg(1);
    std::string destFileName = cmdLine->GetArg(2);
    Directory currentDir = fsh->GetCurrentDirectory();
    currentDir.Move(srcFileName, destFileName);
}

void Commands::Cp(CommandLine *cmdLine, FileSystemHelper *fsh)
{
    if (cmdLine->GetArgCount() < 3) {
        throw std::logic_error("At least one of the files is not specified.");
    }

    std::string srcFileName = cmdLine->GetArg(1);
    std::string destFileName = cmdLine->GetArg(2);
    Directory currentDir = fsh->GetCurrentDirectory();
    currentDir.Copy(srcFileName, destFileName);
}

void Commands::Rm(CommandLine *cmdLine, FileSystemHelper *fsh)
{
    if (cmdLine->GetArgCount() < 2) {
        throw std::logic_error("File or directory is not specified.");
    }

    std::string fileName = cmdLine->GetArg(1);
    Directory currentDir = fsh->GetCurrentDirectory();
    try {
        currentDir.DeleteFile(fileName);
    } catch (const std::ios_base::failure& error) {
        currentDir.DeleteDirectory(fileName);
    }
}

void Commands::Tree(CommandLine *cmdLine, FileSystemHelper *fsh)
{
    if (cmdLine->GetArgCount() > 1) {
        throw std::logic_error("Too many arguments.");
    }

    Directory currentDir = fsh->GetCurrentDirectory();
    TreeStat stat;
    stat.totalDir = 0;
    stat.totalFiles = 0;

    PrintSubTree(currentDir, &stat, 0);

    cout << endl << stat.totalDir << " directories, " << stat.totalFiles << " files." << endl;
}

void Commands::PrintSubTree(Directory& dir, struct TreeStat *stat, int level)
{
    std::vector<Directory> directories = dir.GetDirectories();
    std::vector<File> files = dir.GetFiles();

    std::string gap = "";
    for (size_t i = 0; i < level; ++i) {
        gap += "│   ";
    }

    std::string itemIndent = "├── ";
    std::string lastItemIndent = "└── ";
    for (auto iter = directories.begin(); iter < directories.end(); ++iter) {
        Directory subDir = *iter;
        std::string subDirName = subDir.GetName();
        if (subDirName != "." && subDirName != "..") {
            if (iter + 1 == directories.end() && files.size() == 0) {
                std::cout << gap << lastItemIndent << subDirName << std::endl;
            } else {
                std::cout << gap << itemIndent << subDirName << std::endl;
            }

            PrintSubTree(subDir, stat, level + 1);
        }
    }

    for (auto iter = files.begin(); iter < files.end(); ++iter) {
        File file = *iter;
        if (iter + 1 == files.end()) {
            std::cout << gap << lastItemIndent << file.GetName() << std::endl;
        } else {
            std::cout << gap << itemIndent << file.GetName() << std::endl;
        }
    }

    stat->totalDir += directories.size();
    stat->totalFiles += files.size();
}
