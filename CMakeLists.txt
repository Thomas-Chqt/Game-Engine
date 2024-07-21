# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# Date: 2024/06/20 15:48:52
# ---------------------------------------------------

cmake_minimum_required(VERSION 3.16)

enable_testing()

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(GE_BUILD_EXEMPLES "Build exemples" OFF)
option(GE_BUILD_TESTS    "Build test executable" OFF)

project(Game-Engine)
enable_language(CXX)


add_library(Game-Engine ${GE_LIBRARY_TYPE})
set_target_properties(Game-Engine PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE GAME_ENGINE_SRC "src/*" "include/*")
list(REMOVE_ITEM GAME_ENGINE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main/main.cpp")

target_sources(Game-Engine PRIVATE ${GAME_ENGINE_SRC})
target_include_directories(Game-Engine PRIVATE "src" PUBLIC "include")


add_library(Game-Engine_main ${GE_LIBRARY_TYPE})
set_target_properties(Game-Engine_main PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

target_sources(Game-Engine_main PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/main/main.cpp")
target_include_directories(Game-Engine_main PRIVATE "src" PUBLIC "include")

target_link_libraries(Game-Engine_main PUBLIC Game-Engine)

add_subdirectory(dependencies)
if(GE_BUILD_EXEMPLES)
    add_subdirectory("exemples")
endif()

if (GE_BUILD_TESTS)
    add_subdirectory(tests)
endif()