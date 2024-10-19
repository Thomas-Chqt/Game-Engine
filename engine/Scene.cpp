/*
 * ---------------------------------------------------
 * Scene.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/08 12:10:58
 * ---------------------------------------------------
 */

#include "Scene.hpp"
#include "ECS/Components.hpp"
#include "ECS/ECSWorld.hpp"
#include "ECS/Entity.hpp"
#include "UtilsCPP/String.hpp"
#include <cassert>
#include <string>

namespace GE
{

Scene::Scene(const utils::String& name)
    : m_name(name)
{
}

Entity Scene::activeCamera()
{
    if (m_activeCamera == INVALID_ENTITY_ID)
        return Entity();
    return Entity(m_ecsWorld, m_activeCamera);
}

void Scene::setActiveCamera(const Entity& e)
{
    assert(&e.ecsWorld() == &m_ecsWorld);
    m_activeCamera = e.entityID();
}

Entity Scene::newEntity(const utils::String& name)
{
    Entity newEntity(m_ecsWorld, m_ecsWorld.newEntityID());

    newEntity.emplace<NameComponent>(name);
    newEntity.emplace<TransformComponent>(
        math::vec3f{ 0.0F, 0.0F, 0.0F }, // position
        math::vec3f{ 0.0F, 0.0F, 0.0F }, // rotation
        math::vec3f{ 1.0F, 1.0F, 1.0F }  // scale
    );

    return newEntity;
}

void to_json(nlohmann::json& jsn, const Scene& scene)
{
    jsn["name"] = std::string(scene.name());
    jsn["ecsWorld"] = scene.m_ecsWorld;
    // jsn["activeCamera"] = scene.m_activeCamera;
}

void from_json(const nlohmann::json& jsn, Scene& scene)
{
    auto nameIt = jsn.find("name");        
    scene.m_name = nameIt == jsn.end() ? "no_name" : utils::String(nameIt->template get<std::string>().c_str());

    auto ecsWorldIt = jsn.find("ecsWorld");
    if (ecsWorldIt != jsn.end())
        scene.m_ecsWorld = jsn["ecsWorld"];
    // scene.m_activeCamera = jsn["activeCamera"];
}

}