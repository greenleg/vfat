cmake_minimum_required(VERSION 3.5)

project(vfat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER gcc)

include_directories(.)
include_directories(test)

add_executable(vfat
    alist.h
    alist.c
    binaryreader.h
    binaryreader.c
    cch.h
    cch.c
    cchdir.h
    cchdir.c
    cchfile.h
    cchfile.c
    common.h
    fat.h
    fat.c
    fdisk.h
    fdisk.c
    filesys.h
    filesys.c
    lfnde.h
    lfnde.c
    vbr.h
    vbr.c
    test/minunit.h
    test/testsuite.h
    test/testsuite.c
    test/testfat.c
    test/testcch.c
    test/testcchdir.c
    test/testcchfile.c
    test/testfilesys.c
    main.cpp)
