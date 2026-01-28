# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# Date: 2024/06/20 15:48:52
# ---------------------------------------------------

cmake_minimum_required(VERSION 3.22)

include(cmake/FetchDependencies.cmake)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

project(Game-Engine)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

if (APPLE)
    option(GE_BUILD_METAL "Build the metal backend"   ON)
endif()
option(GE_BUILD_VULKAN   "Build the vulkan backend"   ON)
option(GE_BUILD_TESTS    "Build test executable"      OFF)
option(GE_BUILD_EXAMPLES "Build examples"             OFF)
option(GE_INSTALL        "Enable the install command" ON)

enable_language(CXX)

fetch_dependencies()

add_library(Game-Engine ${GE_LIB_TYPE})

target_compile_features(Game-Engine PUBLIC cxx_std_23)

if(MSVC)
    target_compile_options(Game-Engine PRIVATE /W4)
else()
    target_compile_options(Game-Engine PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(Game-Engine PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-missing-field-initializers>)
endif()

file(GLOB_RECURSE GE_INC "include/*.hpp")
target_sources(Game-Engine PRIVATE ${GFX_INC})

# file(GLOB_RECURSE GE_SRC "*.cpp" "*.hpp")
# target_sources(Game-Engine PRIVATE ${GE_SRC})

target_sources(Game-Engine PRIVATE
    "src/Window.cpp"
    "src/Application.cpp"
    "src/Renderer.cpp"
)

target_include_directories(Game-Engine
    PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

# target_precompile_headers(Graphics PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp")

target_link_libraries(Game-Engine PUBLIC
    Graphics
    glfw
    glm::glm
    imgui
    stb_image
    dlLoad
    assimp::assimp
    nlohmann_json::nlohmann_json
    tinyfiledialogs
    stduuid)

add_subdirectory("editor")

# if(GE_BUILD_EXAMPLES)
#     add_subdirectory("examples/project1")
# endif()
#
# if (GE_BUILD_TESTS)
#     add_subdirectory("tests")
# endif()
