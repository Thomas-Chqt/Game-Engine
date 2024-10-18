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
#include "UtilsCPP/String.hpp"
#include <string>

namespace GE
{

Scene::Scene(const utils::String& name)
    : m_name(name)
{
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
}

void from_json(const nlohmann::json& jsn, Scene& scene)
{
    scene.m_name = utils::String(jsn["name"].template get<std::string>().c_str());
}

}