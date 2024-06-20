# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <thomas.publique@icloud.com>
# Date: 2024/06/20 15:48:52
# ---------------------------------------------------

cmake_minimum_required(VERSION 3.16)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(GE_BUILD_EXEMPLES "Build exemples" OFF)

project(Game-Engine)
enable_language(CXX)
add_library(Game-Engine ${GE_LIBRARY_TYPE})
set_target_properties(Game-Engine PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE GAME_ENGINE_SRC "src/*" "include/*")
target_sources(Game-Engine PRIVATE ${GAME_ENGINE_SRC})
target_include_directories(Game-Engine PRIVATE "src" PUBLIC "include")

add_subdirectory(dependencies)
if(GE_BUILD_EXEMPLES)
    add_subdirectory("exemples")
endif()