#ifndef VFAT_COMMANDS_H
#define VFAT_COMMANDS_H

#include "FileSystemHandle.h"
#include "CommandLine.h"

namespace org::vfat::cli
{
    class Commands
    {
    private:
        static void PrintSubTree(Directory *dir, struct TreeStat *stat, int level);

    public:
        /**
         * @brief Lists all files in the given directory;
         * @param cmdLine
         * @param fsh
         */
        static void Ls(CommandLine *cmdLine, FileSystemHandle *fsh);

        /**
         * @brief Attempts to create a directory;
         * @param cmdLine
         * @param fsh
         */
        static void Mkdir(CommandLine *cmdLine, FileSystemHandle *fsh);

        /**
         * @brief Changes the current directory (the directory in which the user is currently working);
         * @param cmdLine
         * @param fsh
         */
        static void Cd(CommandLine *cmdLine, FileSystemHandle *fsh);

        /**
         * @brief Creates an empty file;
         * @param cmdLine
         * @param fsh
         */
        static void Touch(CommandLine *cmdLine, FileSystemHandle *fsh);

        /**
         * @brief Displays text file on screen;
         * @param cmdLine
         * @param fsh
         */
        static void Cat(CommandLine *cmdLine, FileSystemHandle *fsh);

        /**
         * @brief Imports a file or a directory from native Linux file system;
         * @param cmdLine
         * @param fsh
         */
        static void Import(CommandLine *cmdLine, FileSystemHandle *fsh);

        /**
         * @brief Moves a file or a directory;
         * @param cmdLine
         * @param fsh
         */
        static void Mv(CommandLine *cmdLine, FileSystemHandle *fsh);

        /**
         * @brief Copies a file or a directory;
         * @param cmdLine
         * @param fsh
         */
        static void Cp(CommandLine *cmdLine, FileSystemHandle *fsh);

        /**
         * @brief Deletes a file or a directrory;
         * @param cmdLine
         * @param fsh
         */
        static void Rm(CommandLine *cmdLine, FileSystemHandle *fsh);

        /**
         * @brief Displays the hierarchical structure of the current directory;
         * @param cmdLine
         * @param fsh
         */
        static void Tree(CommandLine *cmdLine, FileSystemHandle *fsh);
    };

    struct TreeStat
    {
        uint32_t totalDir;
        uint32_t totalFiles;
    };
}

#endif // VFAT_COMMANDS_H
