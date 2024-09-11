# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <thomas.publique@icloud.com>
# Date: 2024/06/20 16:24:26
# ---------------------------------------------------

if(NOT TARGET UtilsCPP)
    find_program(GIT_PATH git REQUIRED)
    execute_process(COMMAND ${GIT_PATH} submodule update --init UtilsCPP WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set(UTILSCPP_BUILD_TESTS OFF)
    add_subdirectory(UtilsCPP)
endif()
set_target_properties(UtilsCPP PROPERTIES FOLDER "dependencies")

if(NOT TARGET Math)
    find_program(GIT_PATH git REQUIRED)
    execute_process(COMMAND ${GIT_PATH} submodule update --init Math WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set(MATH_BUILD_TESTS OFF)
    add_subdirectory(Math)
endif()
set_target_properties(Math PROPERTIES FOLDER "dependencies")

if(NOT TARGET Graphics)
    find_program(GIT_PATH git REQUIRED)
    execute_process(COMMAND ${GIT_PATH} submodule update --init Graphics WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set(GFX_BUILD_IMGUI    ON)
    set(GFX_BUILD_EXEMPLES OFF)
    set(GFX_BUILD_METAL    ${GE_BUILD_METAL})
    set(GFX_BUILD_OPENGL   ${GE_BUILD_OPENGL})
    add_subdirectory(Graphics)
endif()
set_target_properties(Graphics PROPERTIES FOLDER "dependencies")
