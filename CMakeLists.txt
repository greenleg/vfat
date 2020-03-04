cmake_minimum_required(VERSION 3.10) # gtest_discover_tests() was introduced in CMake 3.10.

project(vfat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GoogleTest)
enable_testing()

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

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()

set(vfat_lib_src
    include/common.h
    include/alist.h
    include/binaryreader.h
    include/cch.h
    include/cchdir.h
    include/cchfile.h
    include/fat.h
    include/FileDisk.h
    include/filesys.h
    include/lfnde.h
    include/vbr.h
    src/alist.cpp
    src/binaryreader.cpp
    src/cch.cpp
    src/cchdir.cpp
    src/cchfile.cpp
    src/fat.cpp
    src/FileDisk.cpp
    src/filesys.cpp
    src/lfnde.cpp
    src/vbr.cpp)

set(vfat_test_src
    test/BootSectorTest.cpp
    test/FatTest.cpp
    test/ClusterChainTest.cpp
    test/ClusterChainDirectoryTest.cpp
    test/ClusterChainFileTest.cpp
    test/FileSystemTest.cpp
    test/main.cpp)

add_executable(vfat ${vfat_lib_src} src/main.cpp)
add_executable(vfat.test ${vfat_lib_src} ${vfat_test_src})

target_link_libraries(vfat.test gtest)

#gtest_add_tests(TARGET vfat TEST_PREFIX source:)
gtest_discover_tests(vfat.test TEST_PREFIX "runtime:")
