cmake_minimum_required(VERSION 2.8.2)

# Set the name of the project and stores it in the variable PROJECT_NAME.
project(googletest-download NONE)

# Include the ExternalProject module to make 'ExternalProject_Add' available.
include(ExternalProject)

# GoogleTest Project Definition
ExternalProject_Add(googletest
  GIT_REPOSITORY    https://github.com/google/googletest.git
  GIT_TAG           master
  SOURCE_DIR        "${CMAKE_CURRENT_BINARY_DIR}/googletest-src"
  BINARY_DIR        "${CMAKE_CURRENT_BINARY_DIR}/googletest-build"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)
