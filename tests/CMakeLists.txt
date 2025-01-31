# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# Date: 2024/07/21 11:08:32
# ---------------------------------------------------

include(FetchContent)
include(GoogleTest)

add_executable(ECS_test)

set_target_properties(ECS_test PROPERTIES
    CXX_STANDARD          20
    CXX_STANDARD_REQUIRED ON
    FOLDER                "tests"
)

target_sources(ECS_test PRIVATE "ECS_testCases.cpp")

target_include_directories(ECS_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/engine")

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        main
    GIT_SHALLOW    v1.15.2
)
set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)
FetchContent_MakeAvailable(googletest)

set_target_properties(gtest PROPERTIES FOLDER "dependencies")
set_target_properties(gtest_main PROPERTIES FOLDER "dependencies")
target_link_libraries(ECS_test PRIVATE GTest::gtest_main)
target_link_libraries(ECS_test PRIVATE Game-Engine)

gtest_discover_tests(ECS_test)