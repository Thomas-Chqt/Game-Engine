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

#include <cassert>

namespace GE_Editor
{

Project::Project()
{
    GE::Scene& scene = m_scenes.emplace_back("default_scene");

    GE::Entity teapot = scene.newEntity("cube");
    teapot.emplace<GE::TransformComponent>();
    teapot.emplace<GE::MeshComponent>(GE::BUILT_IN_CUBE_ASSET_ID);

    GE::Entity camera = scene.newEntity("camera");
    camera.emplace<GE::TransformComponent>(GE::TransformComponent{
        .position = {  3.0f, 3.0f, 5.0f },
        .rotation = { -0.5f, 0.5f, 0.0f },
        .scale    = {  1.0f, 1.0f, 1.0f }
    });
    camera.emplace<GE::CameraComponent>();
    scene.setActiveCamera(camera);

    GE::Entity light = scene.newEntity("light");
    light.emplace<GE::TransformComponent>();
    light.emplace<GE::LightComponent>();

    camera.addChild(light);

    m_startScene = &scene;
    m_editedScene = &scene;
    m_selectedEntity = teapot;
}

}
