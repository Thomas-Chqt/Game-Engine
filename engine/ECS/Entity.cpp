/*
 * ---------------------------------------------------
 * Entity.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/07 15:32:06
 * ---------------------------------------------------
 */

#include "ECS/Entity.hpp"
#include "ECS/ECSWorld.hpp"

namespace GE
{

Entity::Entity(ECSWorld& world, ECSWorld::EntityID id)
    : m_world(&world), m_entityId(id)
{
}

}