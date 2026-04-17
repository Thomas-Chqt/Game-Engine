/*
 * ---------------------------------------------------
 * Scene.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/08 12:10:58
 * ---------------------------------------------------
 */

#include "Game-Engine/Scene.hpp"

#include <type_traits>
#include <variant>

namespace GE
{

Scene::Scene(AssetManager* assetManager, const std::string& name)
    : m_assetManagerView(assetManager)
    , m_name(name)
{
}

Scene::Scene(AssetManager* assetManager, const Descriptor& desc)
    : m_assetManagerView(assetManager, desc.registredAssets)
    , m_name(desc.name)
    , m_activeCamera(desc.activeCamera)
{
    for (auto& [id, vComponents] : desc.entities) {
        m_ecsWorld.registerEntityID(id);
        for (auto& vComponent : vComponents) {
            std::visit([&](auto& component) {
                m_ecsWorld.emplace<std::remove_cvref_t<decltype(component)>>(id, component);
            }, vComponent);
        }
    }
}

void Scene::setActiveCamera(const Entity& e)
{
    assert(e.world == &m_ecsWorld);
    m_activeCamera = e.entityId;
}

Entity Scene::newEntity(const std::string& name)
{
    Entity newEntity(&m_ecsWorld, m_ecsWorld.newEntityID());
    newEntity.emplace<NameComponent>(name);
    return newEntity;
}

// void to_json(json& jsn, const Scene& scene)
// {
//     jsn["name"] = std::string(scene.name());
//     jsn["ecsWorld"] = scene.m_ecsWorld;
//     if (scene.m_activeCamera != INVALID_ENTITY_ID)
//         jsn["activeCamera"] = scene.m_activeCamera;
//     jsn["assetManager"] = scene.m_assetManager;
// }

// void from_json(const json& jsn, Scene& scene)
// {
//     auto nameIt = jsn.find("name");
//     scene.m_name = nameIt == jsn.end() ? utils::String("no_name") : utils::String(nameIt->template get<std::string>().c_str());

//     auto ecsWorldIt = jsn.find("ecsWorld");
//     if (ecsWorldIt != jsn.end())
//         scene.m_ecsWorld = ecsWorldIt->template get<ECSWorld>();

//     auto activeCameraIt = jsn.find("activeCamera");
//     if (activeCameraIt != jsn.end())
//     {
//         ECSWorld::EntityID activeCameraId = activeCameraIt->template get<ECSWorld::EntityID>();
//         if (scene.ecsWorld().isValidEntityID(activeCameraId) && scene.ecsWorld().has<CameraComponent>(activeCameraId))
//             scene.m_activeCamera = activeCameraId;
//         else
//             scene.m_activeCamera = INVALID_ENTITY_ID;
//     }

//     auto assetManagerIt = jsn.find("assetManager");
//     if (assetManagerIt != jsn.end())
//         scene.m_assetManager = assetManagerIt->template get<AssetManager>();
// }

}
