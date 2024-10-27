# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# Date: 2024/10/24 12:13:43
# ---------------------------------------------------

cmake_minimum_required(VERSION 3.16)

if(NOT PROJECT_NAME)
    message(FATAL_ERROR "PROJECT_NAME not set")
endif()

project(${PROJECT_NAME})
enable_language(CXX)

add_library(${PROJECT_NAME}_scriptLib SHARED)
set_target_properties(${PROJECT_NAME}_scriptLib PROPERTIES 
    CXX_STANDARD              20
    CXX_STANDARD_REQUIRED     ON
    POSITION_INDEPENDENT_CODE ON
    PREFIX                    ""
    DEBUG_POSTFIX             ""
    SUFFIX                    ""
    RUNTIME_OUTPUT_DIRECTORY  $<1:${CMAKE_BINARY_DIR}>
    LIBRARY_OUTPUT_DIRECTORY  $<1:${CMAKE_BINARY_DIR}>
    ARCHIVE_OUTPUT_DIRECTORY  $<1:${CMAKE_BINARY_DIR}>
    # CXX_VISIBILITY_PRESET     hidden
    # VISIBILITY_INLINES_HIDDEN ON
)

file(GLOB_RECURSE SRCS "*.cpp" "*.hpp" "*.h" "*.tpp")
target_sources(${PROJECT_NAME}_scriptLib PRIVATE ${SRCS})

if (INCLUDE_DIRS)
    target_include_directories(${PROJECT_NAME}_scriptLib PRIVATE ${INCLUDE_DIRS})
endif()

target_link_options(${PROJECT_NAME}_scriptLib PRIVATE "-undefined" "dynamic_lookup")
