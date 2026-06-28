# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# ---------------------------------------------------

include(../../cmake/ScriptLibrary.cmake)

file(GLOB_RECURSE BISTRO_SCRIPT_SRCS "${CMAKE_CURRENT_LIST_DIR}/scripts/*.cpp")

ge_add_script_library(bistro_scriptLib ${BISTRO_SCRIPT_SRCS})
target_link_libraries(bistro_scriptLib PRIVATE GE-Editor)

set_target_properties(bistro_scriptLib PROPERTIES
    FOLDER                    "examples"
    PREFIX                    ""
    DEBUG_POSTFIX             ""
    RUNTIME_OUTPUT_DIRECTORY  $<1:${CMAKE_CURRENT_LIST_DIR}>
    LIBRARY_OUTPUT_DIRECTORY  $<1:${CMAKE_CURRENT_LIST_DIR}>
    ARCHIVE_OUTPUT_DIRECTORY  $<1:${CMAKE_CURRENT_LIST_DIR}>
)
