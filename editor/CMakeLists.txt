# ---------------------------------------------------
# CMakeLists.txt
#
# Author: Thomas Choquet <semoir.dense-0h@icloud.com>
# Date: 2024/09/10 10:07:50
# ---------------------------------------------------

add_executable(GE-Editor)
set_target_properties(GE-Editor PROPERTIES ENABLE_EXPORTS ON)

# file(GLOB_RECURSE GE_EDITOR_SRC "*.cpp" "*.hpp")
# target_sources(GE-Editor PRIVATE ${GE_EDITOR_SRC})
target_sources(GE-Editor PRIVATE
    "Editor.cpp"
    "UI/MainMenuBar.cpp"
    "UI/ViewportPanel.cpp"
    # "UI/SceneGraphPanel.cpp"
    "UI/EntityInspectorPanel.cpp"
)

target_include_directories(GE-Editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(GE-Editor PRIVATE "RESOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/resources\"")

target_link_libraries(GE-Editor PRIVATE Game-Engine)
