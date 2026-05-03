# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# Date: 2024/07/21 11:08:32
# ---------------------------------------------------

include(FetchContent)
include(GoogleTest)
include(../cmake/ScriptLibrary.cmake)

add_executable(GE_test)
set_target_properties(GE_test PROPERTIES ENABLE_EXPORTS ON)

target_compile_features(GE_test PUBLIC cxx_std_23)
set_target_properties(GE_test PROPERTIES FOLDER "tests")

file(GLOB SRC "*.cpp")
target_sources(GE_test PRIVATE ${SRC})

target_include_directories(GE_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(GE_test PRIVATE GE_TEST_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources")

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        main
    GIT_SHALLOW    v1.15.2
)
set(BUILD_GMOCK ON)
set(INSTALL_GTEST OFF)
FetchContent_MakeAvailable(googletest)
set_target_properties(gtest PROPERTIES FOLDER "dependencies")
set_target_properties(gtest_main PROPERTIES FOLDER "dependencies")
set_target_properties(gmock PROPERTIES FOLDER "dependencies")
set_target_properties(gmock_main PROPERTIES FOLDER "dependencies")

target_link_libraries(GE_test PRIVATE GTest::gmock GTest::gtest_main Game-Engine)
target_include_directories(GE_test INTERFACE $<TARGET_PROPERTY:Game-Engine,INTERFACE_INCLUDE_DIRECTORIES>)

add_subdirectory(scriptLib)

target_compile_definitions(GE_test PRIVATE GE_TEST_SCRIPT_LIB="$<TARGET_FILE:GE_test_scriptLib>")

gtest_discover_tests(GE_test)

if(APPLE AND NOT CMAKE_GENERATOR STREQUAL "Xcode")
    set(CODESIGN_IDENTITY "-" CACHE STRING "Codesigning identity")

    set(GE_TEST_ENTITLEMENTS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ge-test.entitlements")
    if(EXISTS "${GE_TEST_ENTITLEMENTS_FILE}")
        add_custom_command(TARGET GE_test POST_BUILD
            COMMAND /bin/sh -c "codesign --sign \"${CODESIGN_IDENTITY}\" --force --entitlements \"${GE_TEST_ENTITLEMENTS_FILE}\" \"$<TARGET_FILE:GE_test>\" >/dev/null 2>&1"
            COMMENT "Codesigning GE_test (with entitlements) [identity: ${CODESIGN_IDENTITY}]"
            VERBATIM
        )
    else()
        add_custom_command(TARGET GE_test POST_BUILD
            COMMAND /bin/sh -c "codesign --sign \"${CODESIGN_IDENTITY}\" --force \"$<TARGET_FILE:GE_test>\" >/dev/null 2>&1"
            COMMENT "Codesigning GE_test [identity: ${CODESIGN_IDENTITY}]"
            VERBATIM
        )
    endif()
endif()
