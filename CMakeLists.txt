# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# Date: 2024/06/20 15:48:52
# ---------------------------------------------------

cmake_minimum_required(VERSION 3.16)

include(FetchContent)

enable_testing()
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build using shared libraries" FORCE)

option(GE_BUILD_TESTS    "Build test executable" OFF)
option(GE_BUILD_EXAMPLES "Build example scripts" OFF)

if (APPLE)
    option(GE_BUILD_METAL  "Build with metal enable"  ON)
    option(GE_BUILD_OPENGL "Build with OpenGL enable" ON)
else()
    option(GE_BUILD_OPENGL "Build with OpenGL enable" ON)
endif()

if (NOT GE_BUILD_METAL AND NOT GE_BUILD_OPENGL)
    message(FATAL_ERROR "One graphic api must be enable")
endif()

project(Game-Engine)
enable_language(CXX)

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

add_subdirectory("dependencies")
add_subdirectory("engine")
add_subdirectory("editor")

if(GE_BUILD_EXAMPLES)
    add_subdirectory("examples/project1")
endif()

if (GE_BUILD_TESTS)
    add_subdirectory("tests")
endif()