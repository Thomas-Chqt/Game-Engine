/*
 * ---------------------------------------------------
 * Entity.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/07 15:32:06
 * ---------------------------------------------------
 */

#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/ECSWorld.hpp"
#include "Scene/InternalComponents.hpp"
#include "Math/Vector.hpp"

namespace GE
{

Entity::Entity(ECSWorld& world, ECSWorld::EntityID id)
    : m_world(&world), m_entityId(id)
{
}

utils::String& Entity::name()
{
    return get<NameComponent>().name;
}

math::mat4x4 Entity::transform()
{
    return get<TransformComponent>().transform();
}

math::vec3f& Entity::position()
{
    return get<TransformComponent>().position;
}

math::vec3f& Entity::rotation()
{
    return get<TransformComponent>().rotation;
}

math::vec3f& Entity::scale()
{
    return get<TransformComponent>().scale;
}

}