# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# Date: 2024/11/13 20:48:06
# ---------------------------------------------------

cmake_minimum_required(VERSION 3.16)

add_library(project1_scriptLib SHARED)
set_target_properties(project1_scriptLib PROPERTIES 
    CXX_STANDARD              20
    CXX_STANDARD_REQUIRED     ON
    FOLDER                    "examples"
    PREFIX                    ""
    DEBUG_POSTFIX             ""
    RUNTIME_OUTPUT_DIRECTORY  $<1:${CMAKE_CURRENT_LIST_DIR}>
    LIBRARY_OUTPUT_DIRECTORY  $<1:${CMAKE_CURRENT_LIST_DIR}>
    ARCHIVE_OUTPUT_DIRECTORY  $<1:${CMAKE_CURRENT_LIST_DIR}>
)

if(NOT WIN32)
    set_target_properties(project1_scriptLib PROPERTIES SUFFIX "")
endif()

file(GLOB_RECURSE SRCS "${CMAKE_CURRENT_LIST_DIR}/scripts/*")
target_sources(project1_scriptLib PRIVATE ${SRCS})

target_compile_definitions(project1_scriptLib PRIVATE "PROJECT1_API_EXPORT" INTERFACE "PROJECT1_API_IMPORT")

if(APPLE)
    target_link_options(project1_scriptLib PRIVATE "-undefined" "dynamic_lookup")
endif()

target_link_libraries(project1_scriptLib PRIVATE GE-Editor)