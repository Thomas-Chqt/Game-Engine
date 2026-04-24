/*
 * ---------------------------------------------------
 * Project.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Project.hpp"

#include <Game-Engine/AssetManager.hpp>
#include <Game-Engine/Scene.hpp>
#include <Game-Engine/Components.hpp>
#include <Game-Engine/ECSWorld.hpp>
#include <Game-Engine/Mesh.hpp>
#include <Game-Engine/AssetManagerView.hpp>

#include <imgui.h>

#include <cassert>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

namespace  {
    constexpr std::string_view DEFAULT_IMGUI_SETTINGS =
        "[Window][WindowOverViewport_11111111]\n"
        "Pos=0,19\nSize=1280,701\nCollapsed=0\n\n"
        "[Window][Debug##Default]\n"
        "Pos=60,60\nSize=400,400\nCollapsed=0\n\n[Window][viewport]\n"
        "Pos=0,19\nSize=1004,525\nCollapsed=0\nDockId=0x00000001,0\n\n"
        "[Window][Scene graph]\n"
        "Pos=1006,19\nSize=274,273\nCollapsed=0\nDockId=0x00000005,0\n\n"
        "[Window][Entity inspector]\n"
        "Pos=1006,294\nSize=274,426\nCollapsed=0\nDockId=0x00000006,0\n\n"
        "[Window][Scenes]\n"
        "Pos=0,546\nSize=1004,174\nCollapsed=0\nDockId=0x00000002,0\n\n"
        "[Window][Project properties]\n"
        "Pos=435,238\nSize=409,244\nCollapsed=0\n\n"
        "[Docking][Data]\n"
        "DockSpace     ID=0x08BD597D Window=0x1BBC0F80 Pos=0,19 Size=1280,701 Split=X\n"
        "  DockNode    ID=0x00000003 Parent=0x08BD597D SizeRef=1004,701 Split=Y\n"
        "    DockNode  ID=0x00000001 Parent=0x00000003 SizeRef=1280,525 CentralNode=1 Selected=0xF6034841\n"
        "    DockNode  ID=0x00000002 Parent=0x00000003 SizeRef=1280,174 Selected=0xC73B27C0\n"
        "  DockNode    ID=0x00000004 Parent=0x08BD597D SizeRef=274,701 Split=Y Selected=0x848E2614\n"
        "    DockNode  ID=0x00000005 Parent=0x00000004 SizeRef=142,273 Selected=0x2C1AB1E4\n"
        "    DockNode  ID=0x00000006 Parent=0x00000004 SizeRef=142,426 Selected=0x848E2614\n"
        "\n";
}

namespace GE_Editor
{

Project::Project()
    : m_name("Untitled Project")
    , m_startScene(0)
    , m_imguiSettings(DEFAULT_IMGUI_SETTINGS)
{
    auto [it, inserted] = m_scenes.emplace(0, GE::Scene::Descriptor());
    assert(inserted);
    GE::Scene::Descriptor& desc = it->second;

    desc.name = "default_scene";
    desc.activeCamera = 1;
    desc.registredAssets = {
        { GE::BUILT_IN_CUBE_ASSET_ID, GE::AssetPath<GE::Mesh>() }
    };
    desc.entities = {
        {
            0, {
               GE::NameComponent{"cube"},
               GE::TransformComponent{},
               GE::MeshComponent{GE::BUILT_IN_CUBE_ASSET_ID}
            }
        },
        {
            1, {
               GE::NameComponent{"camera"},
               GE::HierarchyComponent{ .firstChild = 2 },
               GE::TransformComponent{
                   .position = {  3.0f, 3.0f, 5.0f },
                   .rotation = { -0.5f, 0.5f, 0.0f },
                   .scale    = {  1.0f, 1.0f, 1.0f }
               },
               GE::CameraComponent{}
            }
        },
        {
            2, {
               GE::NameComponent{"light"},
               GE::HierarchyComponent{ .parent = 1 },
               GE::TransformComponent{},
               GE::LightComponent{}
            }
        }
    };

    m_startScene = it->first;
}

GE::Game::Descriptor Project::makeGameDescriptor() const
{
    return {
        .scenes = m_scenes
                  | std::views::values
                  | std::views::transform([](const GE::Scene::Descriptor& sceneDescriptor) {
                        return std::make_pair(sceneDescriptor.name, sceneDescriptor);
                    })
                  | std::ranges::to<std::map<std::string, GE::Scene::Descriptor>>(),
        .activeScene = startScene().second.name
    };
}

}
