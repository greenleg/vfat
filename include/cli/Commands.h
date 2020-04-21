#ifndef VFAT_COMMANDS_H
#define VFAT_COMMANDS_H

#include "FileSystemHandle.h"
#include "CommandLine.h"

namespace org::vfat::cli
{
    class Commands
    {
    public:
        static void ls(CommandLine *cmdLine, FileSystemHandle *fsh);
        static void mkdir(CommandLine *cmdLine, FileSystemHandle *fsh);
        static void cd(CommandLine *cmdLine, FileSystemHandle *fsh);
    };
}

#endif // VFAT_COMMANDS_H
