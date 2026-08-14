cmake_minimum_required(VERSION 3.30)

project(CDLib)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Prevent the user from polluting the root directory with build files
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()


include("cmake/get_cpm.cmake")
include("cmake/thirdparty.cmake")


add_subdirectory(CDLib)
