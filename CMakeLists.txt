cmake_minimum_required(VERSION 3.5)

project(vfat LANGUAGES CXX)

#add_compile_definitions(RUN_ALL_TEST)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Download and unpack googletest at configure time
configure_file(gtest/CMakeLists.txt googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

message("gtest_SOURCE_DIR=${gtest_SOURCE_DIR}")

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()

add_executable(vfat
#    include/common.h
#    include/alist.h
#    include/binaryreader.h
#    include/cch.h
#    include/cchdir.h
#    include/cchfile.h
#    include/fat.h
#    include/fdisk.h
#    include/filesys.h
#    include/lfnde.h
#    include/vbr.h
#    src/alist.cpp
#    src/binaryreader.cpp
#    src/cch.cpp
#    src/cchdir.cpp
#    src/cchfile.cpp
#    src/fat.cpp
#    src/fdisk.cpp
#    src/filesys.cpp
#    src/lfnde.cpp
#    src/vbr.cpp
#    test/minunit.h
#    test/testsuite.h
#    test/testvbr.cpp
#    test/testfat.cpp
#    test/testcch.cpp
#    test/testcchdir.cpp
#    test/testcchfile.cpp
#    test/testfilesys.cpp
    main.cpp)

target_link_libraries(vfat gtest_main)
#add_test()

