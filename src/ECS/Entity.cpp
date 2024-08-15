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
#include "ECS/InternalComponents.hpp"
#include "Math/Vector.hpp"
#include <cassert>

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

Entity Entity::parent()
{
    return get<HierarchicalComponent>().parent;
}

Entity Entity::firstChild()
{
    return get<HierarchicalComponent>().firstChild;
}

Entity Entity::nextChild()
{
    return get<HierarchicalComponent>().nextChild;
}

void Entity::pushChild(Entity child)
{
    HierarchicalComponent* comp;
    if (has<HierarchicalComponent>() == false)
        comp = &emplace<HierarchicalComponent>();
    else
        comp = &get<HierarchicalComponent>();

    HierarchicalComponent* childComp;
    if (child.has<HierarchicalComponent>() == false)
        childComp = &child.emplace<HierarchicalComponent>();
    else
        childComp = &child.get<HierarchicalComponent>();
    
    assert(childComp->parent == false);
    childComp->parent = *this;
    childComp->nextChild = comp->firstChild;
    comp->firstChild = child;
}

Entity Entity::popChild()
{
    assert(has<HierarchicalComponent>());

    HierarchicalComponent& comp = get<HierarchicalComponent>();

    Entity prevFirstChild = comp.firstChild;
    HierarchicalComponent& prevFirstChildComp = prevFirstChild.get<HierarchicalComponent>();

    comp.firstChild = prevFirstChildComp.nextChild;
    prevFirstChildComp.parent = Entity();
    prevFirstChildComp.nextChild = Entity();
    return prevFirstChild;
}

math::mat4x4 Entity::worldTransform()
{
    if (has<HierarchicalComponent>() == false || parent() == false)
        return transform();
    return parent().worldTransform() * transform();
}

}