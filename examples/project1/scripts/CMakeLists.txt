# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# Date: 2024/10/24 12:13:43
# ---------------------------------------------------

cmake_minimum_required(VERSION 3.16)

enable_language(CXX)

add_library(project1 SHARED)
set_target_properties(project1
    PROPERTIES CXX_STANDARD   20
    CXX_STANDARD_REQUIRED     ON
    FOLDER                    "examples"
    POSITION_INDEPENDENT_CODE ON
    # CXX_VISIBILITY_PRESET     hidden
    # VISIBILITY_INLINES_HIDDEN ON
)

file(GLOB_RECURSE SRCS "*.cpp" "*.hpp" "*.h" "*.tpp")
target_sources(project1 PRIVATE ${SRCS})
target_include_directories(project1 PRIVATE
    $<TARGET_PROPERTY:Game-Engine,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:UtilsCPP,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Math,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Graphics,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:nlohmann_json,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:stduuid,INCLUDE_DIRECTORIES>
)
# target_link_options(project1 PRIVATE "-undefined" "dynamic_lookup")
