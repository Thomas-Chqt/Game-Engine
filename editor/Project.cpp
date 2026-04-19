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

#include <cassert>
#include <string>
#include <utility>

namespace GE_Editor
{

Project::Project()
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

GE::Game::Descriptor Project::gameDescriptor() const
{
    std::map<std::string, GE::Scene::Descriptor> scenes;
    for (const auto& [sceneId, sceneDesc] : m_scenes)
        scenes.insert(std::make_pair(sceneDesc.name, sceneDesc));

    return {
        .scenes = scenes,
        .activeScene = startScene().second.name
    };
}

}
