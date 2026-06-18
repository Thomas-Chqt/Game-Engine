/*
 * ---------------------------------------------------
 * Project.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Project.hpp"

#include <Game-Engine/Components.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/Mesh.hpp>
#include <Game-Engine/Scene.hpp>

#include <glm/gtc/quaternion.hpp>
#include <imgui.h>

#include <string>
#include <string_view>

namespace
{

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

GE::Scene::Descriptor makeDefaultCubeScene(std::string_view name)
{
    GE::Scene::Descriptor scene;
    scene.name = std::string(name);

    GE::Entity cube{&scene.ecsWorld, scene.ecsWorld.newEntityID()};
    cube.emplace<GE::NameComponent>("cube");
    cube.emplace<GE::TransformComponent>();
    cube.emplace<GE::MeshComponent>(GE::BUILT_IN_CUBE_ID);

    GE::Entity camera{&scene.ecsWorld, scene.ecsWorld.newEntityID()};
    camera.emplace<GE::NameComponent>("camera");
    auto& transformComponent = camera.emplace<GE::TransformComponent>();
    transformComponent.position = {  3.0f, 3.0f, 5.0f };
    transformComponent.rotation = glm::quat(glm::vec3{ -0.5f, 0.5f, 0.0f });
    transformComponent.scale    = {  1.0f, 1.0f, 1.0f };
    camera.emplace<GE::CameraComponent>();

    GE::Entity light{&scene.ecsWorld, scene.ecsWorld.newEntityID()};
    light.emplace<GE::NameComponent>("light");
    light.emplace<GE::TransformComponent>();
    light.emplace<GE::LightComponent>();

    camera.addChild(light);

    scene.activeCameraId = camera.entityId;

    return scene;
}

}

namespace GE_Editor
{

Project makeDefaultProject()
{
    return Project{
        .name = "Untitled Project",
        .scenes = { makeDefaultCubeScene("default_scene") },
        .startSceneName = "default_scene",
        .editorCamera = {{3.0f, 3.0f, 5.0f}, glm::quat(glm::vec3{-0.5f, 0.5f, 0.0f})},
        .editedSceneName = "default_scene",
        .imguiSettings = std::string(DEFAULT_IMGUI_SETTINGS)
    };
}

}
