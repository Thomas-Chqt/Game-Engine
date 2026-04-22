# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# Date: 2024/09/10 10:07:50
# ---------------------------------------------------

add_executable(GE-Editor)
set_target_properties(GE-Editor PROPERTIES
    ENABLE_EXPORTS ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)

file(GLOB_RECURSE GE_EDITOR_SRC "*.cpp" "*.hpp")
target_sources(GE-Editor PRIVATE ${GE_EDITOR_SRC})

target_include_directories(GE-Editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(GE-Editor PRIVATE "RESOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/resources\"")

target_link_libraries(GE-Editor PRIVATE Game-Engine)
target_include_directories(GE-Editor INTERFACE $<TARGET_PROPERTY:Game-Engine,INTERFACE_INCLUDE_DIRECTORIES>)

if(APPLE)
    target_link_options(GE-Editor PRIVATE "LINKER:-force_load,$<TARGET_FILE:Game-Engine>")
elseif(NOT MSVC)
    target_link_options(GE-Editor PRIVATE "LINKER:--whole-archive,$<TARGET_FILE:Game-Engine>,--no-whole-archive")
endif()
