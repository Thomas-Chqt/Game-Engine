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

option(GE_BUILD_EXEMPLES "Build exemples"                     OFF)
option(GE_BUILD_TESTS    "Build test executable"              OFF)
option(GE_INSTALL        "Enable Game Engine install command" ON)

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

FetchContent_Declare(assimp GIT_REPOSITORY https://github.com/assimp/assimp.git GIT_TAG v5.4.1 GIT_SHALLOW 1)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_NO_EXPORT OFF)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_OBJ_IMPORTER ON)
set(ASSIMP_BUILD_FBX_IMPORTER ON)
set(ASSIMP_BUILD_GLTF_IMPORTER ON)
FetchContent_MakeAvailable(assimp)
set_target_properties(assimp PROPERTIES FOLDER "dependencies")

FetchContent_Declare(crossguid GIT_REPOSITORY https://github.com/graeme-hill/crossguid.git GIT_SHALLOW 1)
set(CROSSGUID_TESTS OFF)
FetchContent_MakeAvailable(crossguid)
set_target_properties(crossguid PROPERTIES FOLDER "dependencies")

FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.11.3 GIT_SHALLOW 1)
set(JSON_BuildTests OFF)
set(JSON_Install OFF)
FetchContent_MakeAvailable(json)

add_subdirectory("engine")
add_subdirectory("editor")

if(GE_BUILD_EXEMPLES)
    add_subdirectory("exemples")
endif()

if (GE_BUILD_TESTS)
    add_subdirectory("tests")
endif()

if(GE_INSTALL)
    install(TARGETS Game-Engine
        RUNTIME DESTINATION "bin"
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
    )
    install(DIRECTORY "include/" DESTINATION "include")

    install(TARGETS assimp
        RUNTIME DESTINATION "bin"
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
    )
endif()