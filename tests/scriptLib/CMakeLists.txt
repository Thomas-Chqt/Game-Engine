# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# ---------------------------------------------------

include(../../cmake/ScriptLibrary.cmake)

ge_add_script_library(GE_test_scriptLib "TestScript.cpp")
target_link_libraries(GE_test_scriptLib PRIVATE GE_test)

set_target_properties(GE_test_scriptLib PROPERTIES
    FOLDER "tests"
    PREFIX ""
    DEBUG_POSTFIX ""
    RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_CURRENT_BINARY_DIR}>
    LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_CURRENT_BINARY_DIR}>
    ARCHIVE_OUTPUT_DIRECTORY $<1:${CMAKE_CURRENT_BINARY_DIR}>
)
