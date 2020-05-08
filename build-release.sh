#!/bin/bash

# Jump to the project directory;
VFAT_DIR="`dirname \"$0\"`"
cd "$VFAT_DIR"

# Prepare Makefile;
SOURCE_DIR="."
BINARY_DIR="../build-vfat-Release"
cmake -S "$SOURCE_DIR" -B "$BINARY_DIR" -D BUILD_GMOCK:BOOL=OFF -D CMAKE_BUILD_TYPE:STRING=Release -D INSTALL_GTEST:BOOL=OFF

# Build Makefile;
cd "$BINARY_DIR"
make
