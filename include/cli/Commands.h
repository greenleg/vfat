#ifndef VFAT_COMMANDS_H
#define VFAT_COMMANDS_H

#include "FileSystemHelper.h"
#include "CommandLine.h"

namespace org::vfat::cli
{
    class Commands
    {
    private:
        static void PrintSubTree(Directory& dir, struct TreeStat *stat, int level);

    public:
        /**
         * @brief Lists all files in the given directory;
         * @param cmdLine
         * @param fsh
         */
        static void Ls(const CommandLine& cmdLine, FileSystemHelper *fsh);

        /**
         * @brief Attempts to create a directory;
         * @param cmdLine
         * @param fsh
         */
        static void Mkdir(const CommandLine& cmdLine, FileSystemHelper *fsh);

        /**
         * @brief Changes the current directory (the directory in which the user is currently working);
         * @param cmdLine
         * @param fsh
         */
        static void Cd(const CommandLine& cmdLine, FileSystemHelper *fsh);

        /**
         * @brief Creates an empty file;
         * @param cmdLine
         * @param fsh
         */
        static void Touch(const CommandLine& cmdLine, FileSystemHelper *fsh);

        /**
         * @brief Displays text file on screen;
         * @param cmdLine
         * @param fsh
         */
        static void Cat(const CommandLine& cmdLine, FileSystemHelper *fsh);

        /**
         * @brief Imports a file or a directory from native Linux file system;
         * @param cmdLine
         * @param fsh
         */
        static void Import(const CommandLine& cmdLine, FileSystemHelper *fsh);

        /**
         * @brief Moves a file or a directory;
         * @param cmdLine
         * @param fsh
         */
        static void Mv(const CommandLine& cmdLine, FileSystemHelper *fsh);

        /**
         * @brief Copies a file or a directory;
         * @param cmdLine
         * @param fsh
         */
        static void Cp(const CommandLine& cmdLine, FileSystemHelper *fsh);

        /**
         * @brief Deletes a file or a directrory;
         * @param cmdLine
         * @param fsh
         */
        static void Rm(const CommandLine& cmdLine, FileSystemHelper *fsh);

        /**
         * @brief Displays the hierarchical structure of the current directory;
         * @param cmdLine
         * @param fsh
         */
        static void Tree(const CommandLine& cmdLine, FileSystemHelper *fsh);
    };

    struct TreeStat
    {
        uint32_t totalDir;
        uint32_t totalFiles;
    };
}

#endif // VFAT_COMMANDS_H
