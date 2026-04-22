# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# ---------------------------------------------------

include(../../cmake/ScriptLibrary.cmake)

file(GLOB_RECURSE PROJECT1_SCRIPT_SRCS "${CMAKE_CURRENT_LIST_DIR}/scripts/*.cpp")

ge_add_script_library(project1_scriptLib ${PROJECT1_SCRIPT_SRCS})
target_link_libraries(project1_scriptLib PRIVATE GE-Editor)

set_target_properties(project1_scriptLib PROPERTIES
    FOLDER                    "examples"
    PREFIX                    ""
    DEBUG_POSTFIX             ""
    RUNTIME_OUTPUT_DIRECTORY  $<1:${CMAKE_CURRENT_LIST_DIR}>
    LIBRARY_OUTPUT_DIRECTORY  $<1:${CMAKE_CURRENT_LIST_DIR}>
    ARCHIVE_OUTPUT_DIRECTORY  $<1:${CMAKE_CURRENT_LIST_DIR}>
)
