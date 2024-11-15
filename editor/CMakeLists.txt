# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# Date: 2024/09/10 10:07:50
# ---------------------------------------------------

add_executable(GE-Editor)
set_target_properties(GE-Editor
    PROPERTIES CXX_STANDARD 20
    CXX_STANDARD_REQUIRED   ON
    ENABLE_EXPORTS          ON
)

file(GLOB_RECURSE GE_EDITOR_SRC "*.cpp" "*.hpp")
target_sources(GE-Editor PRIVATE ${GE_EDITOR_SRC})

target_include_directories(GE-Editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(GE-Editor PRIVATE "RESSOURCES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/ressources\"")
target_compile_definitions(GE-Editor INTERFACE "UTILSCPP_API_IMPORT" "MATH_API_IMPORT" "GAME_ENGINE_API_IMPORT")

target_link_libraries(GE-Editor PRIVATE Game-Engine)
target_include_directories(GE-Editor INTERFACE $<TARGET_PROPERTY:Game-Engine,INTERFACE_INCLUDE_DIRECTORIES>)

target_link_libraries(GE-Editor PRIVATE tinyfiledialogs)