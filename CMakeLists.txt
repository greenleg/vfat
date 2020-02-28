cmake_minimum_required(VERSION 3.5)

project(vfat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_C_COMPILER gcc)

add_executable(vfat
    include/common.h
    include/alist.h
    include/binaryreader.h
    include/cch.h
    include/cchdir.h
    include/cchfile.h
    include/fat.h
    include/fdisk.h
    include/filesys.h
    include/lfnde.h
    include/vbr.h
    src/alist.cpp
    src/binaryreader.cpp
    src/cch.cpp
    src/cchdir.cpp
    src/cchfile.cpp
    src/fat.cpp
    src/fdisk.cpp
    src/filesys.cpp
    src/lfnde.cpp
    src/vbr.cpp
    test/minunit.h
    test/testsuite.h
    test/testvbr.cpp
    test/testfat.cpp
    test/testcch.cpp
    test/testcchdir.cpp
    test/testcchfile.cpp
    test/testfilesys.cpp
    main.cpp)
