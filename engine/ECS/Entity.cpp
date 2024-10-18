/*
 * ---------------------------------------------------
 * Entity.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/07 15:32:06
 * ---------------------------------------------------
 */

#include "ECS/Entity.hpp"
#include "ECS/Components.hpp"
#include "ECS/ECSWorld.hpp"
#include <cassert>

namespace GE
{

Entity::Entity(ECSWorld& world, ECSWorld::EntityID id)
    : m_world(&world), m_entityId(id)
{
}

utils::String& Entity::name()
{
    return get<NameComponent>();
}

Entity Entity::parent()
{
    return get<HierarchyComponent>().parent;
}

Entity Entity::firstChild()
{
    return get<HierarchyComponent>().firstChild;
}

Entity Entity::nextChild()
{
    return get<HierarchyComponent>().nextChild;
}

Entity Entity::lastChild()
{
    Entity curr = firstChild();
    while (curr && curr.nextChild())
        curr = curr.nextChild();
    return curr;
}

void Entity::addChild(Entity child)
{
    addChild(child, lastChild());
}

void Entity::addChild(Entity child, Entity after)
{
    assert(m_world == &child.ecsWorld());

    if (has<HierarchyComponent>() == false)
        emplace<HierarchyComponent>();
    if (child.has<HierarchyComponent>() == false)
        child.emplace<HierarchyComponent>();

    assert(child.parent() == false);
    assert((!after && !firstChild()) || (after && firstChild())); // ensure we not asking to add the child after a certain entity if there is no child

    if (!after && !firstChild())
    {
        child.nextChild() = Entity();
        firstChild() = child;
    }
    else
    {
        child.nextChild() = after.nextChild();
        after.nextChild() = child;
    }
}

void Entity::removeChild(Entity child)
{
    assert(m_world == &child.ecsWorld());

    if (child == firstChild())
        firstChild() = firstChild().nextChild();
    else
    {
        Entity curr = firstChild();
        while (curr.nextChild() != child)
            curr = curr.nextChild();
        curr.nextChild() = curr.nextChild().nextChild();
    }

    child.parent() = Entity();
    child.nextChild() = Entity();
}

math::mat4x4 Entity::transform()
{
    return get<TransformComponent>();
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