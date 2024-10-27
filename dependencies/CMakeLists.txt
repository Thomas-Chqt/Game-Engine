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

FetchContent_Declare(assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v5.4.1
    GIT_SHALLOW    1
)
set(ASSIMP_BUILD_TESTS                    OFF)
set(ASSIMP_INSTALL                        OFF)
set(ASSIMP_NO_EXPORT                      ON)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_OBJ_IMPORTER             ON)
set(ASSIMP_BUILD_FBX_IMPORTER             ON)
set(ASSIMP_BUILD_GLTF_IMPORTER            ON)
FetchContent_MakeAvailable(assimp)
set_target_properties(assimp PROPERTIES FOLDER "dependencies")


FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    1
)
set(JSON_BuildTests OFF)
set(JSON_Install    OFF)
FetchContent_MakeAvailable(json)

FetchContent_Declare(tinyfiledialogs
    GIT_REPOSITORY https://github.com/Thomas-Chqt/tinyfiledialogs.git
    GIT_TAG        0806552c78bf70658628e0ca4e3b962ca163ac70
)
set(TFD_BUILD_EXAMPLES OFF)
set(TFD_INSTALL        OFF)
FetchContent_MakeAvailable(tinyfiledialogs)
set_target_properties(tinyfiledialogs PROPERTIES FOLDER "dependencies")

FetchContent_Declare(stduuid
    GIT_REPOSITORY https://github.com/mariusbancila/stduuid.git
    GIT_TAG        v1.2.3
    GIT_SHALLOW    1
)
set(UUID_BUILD_TESTS      OFF)
set(UUID_SYSTEM_GENERATOR OFF)
set(UUID_TIME_GENERATOR   OFF)
set(UUID_USING_CXX20_SPAN ON)
set(UUID_ENABLE_INSTALL   OFF)
FetchContent_MakeAvailable(stduuid)
set_target_properties(stduuid PROPERTIES FOLDER "dependencies")