/*
 * ---------------------------------------------------
 * Scene.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/07 22:01:50
 * ---------------------------------------------------
 */

#include "Game-Engine/Scene.hpp"
#include "ECS/ECSView.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/Entity.hpp"
#include "Scene/InternalComponents.hpp"
#include "UtilsCPP/String.hpp"

namespace GE
{

Entity Scene::newEntity(const utils::String& name)
{
    Entity newEntity = Entity(m_world, m_world.newEntity());
    newEntity.emplace<NameComponent>(name);
    newEntity.emplace<TransformComponent>(
        math::vec3f{ 0.0F, 0.0F, 0.0F }, // position
        math::vec3f{ 0.0F, 0.0F, 0.0F }, // rotation
        math::vec3f{ 1.0F, 1.0F, 1.0F }  // scale
    );
    return newEntity;
}

void Scene::onUpdate()
{
    ECSView<ScriptComponent>(m_world).onEach([](Entity, ScriptComponent& scriptComponent){
        scriptComponent.instance->onUpdate();
    });
}

}