# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# Date: 2024/07/21 11:08:32
# ---------------------------------------------------

include(FetchContent)
include(GoogleTest)

add_executable(Game-Engine_test)

set_target_properties(Game-Engine_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    FOLDER "tests"
)


file(GLOB_RECURSE UTILSCPP_TEST_SRC "*.cpp" "*.tpp" "*.hpp")
target_sources(Game-Engine_test PRIVATE ${UTILSCPP_TEST_SRC})

target_include_directories(Game-Engine_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/src")


target_link_libraries(Game-Engine_test PRIVATE Game-Engine)

set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        main
    GIT_SHALLOW    v1.15.2
)
FetchContent_MakeAvailable(googletest)
set_target_properties(gtest PROPERTIES FOLDER "dependencies")
set_target_properties(gtest_main PROPERTIES FOLDER "dependencies")
target_link_libraries(Game-Engine_test PRIVATE GTest::gtest_main)


gtest_discover_tests(Game-Engine_test)