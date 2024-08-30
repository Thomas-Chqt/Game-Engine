# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# Date: 2024/06/20 15:48:52
# ---------------------------------------------------

cmake_minimum_required(VERSION 3.16)

include(FetchContent)

enable_testing()

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

if(GE_BUILD_METAL)
    set(MTL_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
    set(MTL_SHADER_LIB "${CMAKE_CURRENT_BINARY_DIR}/MTLShaderLib.metallib")
endif()
if(GE_BUILD_OPENGL)
    set(OPENGL_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
endif()

project(Game-Engine)
enable_language(CXX)


add_library(Game-Engine)
set_target_properties(Game-Engine PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE GAME_ENGINE_SRC "src/*" "include/*")
target_sources(Game-Engine PRIVATE ${GAME_ENGINE_SRC})

target_include_directories(Game-Engine PRIVATE "src" PUBLIC "include")

if(GE_BUILD_METAL)
    target_compile_definitions(Game-Engine PRIVATE "MTL_SHADER_LIB=\"${MTL_SHADER_LIB}\"")
endif()
if(GE_BUILD_OPENGL)
    target_compile_definitions(Game-Engine PRIVATE "OPENGL_SHADER_DIR=\"${OPENGL_SHADER_DIR}\"")
endif()

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
target_link_libraries(Game-Engine PRIVATE Graphics assimp)

if(GE_BUILD_METAL)
    add_custom_target(MTLShaderLib DEPENDS ${MTL_SHADER_LIB})

    file(GLOB MTL_SHADER_LIB_SRCS "${MTL_SHADER_DIR}/*.metal")
    target_sources(MTLShaderLib PRIVATE ${MTL_SHADER_LIB_SRCS})

    add_dependencies(Game-Engine MTLShaderLib)
    
    if(CMAKE_GENERATOR STREQUAL "Xcode")
        list(APPEND MTL_SHADER_BUILD_FLAGS "-gline-tables-only" "-frecord-sources")
    endif()
    
    find_program(XCRUN_PATH xcrun REQUIRED)
    
    foreach(src IN LISTS MTL_SHADER_LIB_SRCS)
        string(REPLACE ".metal" ".air" air "${src}")
        string(REPLACE ${MTL_SHADER_DIR} ${CMAKE_CURRENT_BINARY_DIR} air "${air}")
        add_custom_command(OUTPUT "${air}" COMMAND ${XCRUN_PATH} -sdk macosx metal ${MTL_SHADER_BUILD_FLAGS} -o "${air}" -c "${src}" DEPENDS "${src}")
        list(APPEND MTL_SHADER_LIB_AIRS "${air}")
    endforeach()

    add_custom_command(OUTPUT ${MTL_SHADER_LIB} COMMAND ${XCRUN_PATH} -sdk macosx metallib ${MTL_SHADER_LINK_FLAGS} -o ${MTL_SHADER_LIB} ${MTL_SHADER_LIB_AIRS} DEPENDS ${MTL_SHADER_LIB_AIRS})
endif()

add_subdirectory(dependencies)
if(GE_BUILD_EXEMPLES)
    add_subdirectory("exemples")
endif()

if (GE_BUILD_TESTS)
    add_subdirectory(tests)
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