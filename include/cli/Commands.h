#ifndef VFAT_COMMANDS_H
#define VFAT_COMMANDS_H

#include "FileSystemHandle.h"
#include "CommandLine.h"

namespace org::vfat::cli
{
    class Commands
    {
    public:
        static void Ls(CommandLine *cmdLine, FileSystemHandle *fsh);
        static void Mkdir(CommandLine *cmdLine, FileSystemHandle *fsh);
        static void Cd(CommandLine *cmdLine, FileSystemHandle *fsh);
        static void Touch(CommandLine *cmdLine, FileSystemHandle *fsh);
        static void Cat(CommandLine *cmdLine, FileSystemHandle *fsh);
        static void Import(CommandLine *cmdLine, FileSystemHandle *fsh);
        static void Mv(CommandLine *cmdLine, FileSystemHandle *fsh);
        static void Cp(CommandLine *cmdLine, FileSystemHandle *fsh);
        static void Rm(CommandLine *cmdLine, FileSystemHandle *fsh);
    };
}

#endif // VFAT_COMMANDS_H
