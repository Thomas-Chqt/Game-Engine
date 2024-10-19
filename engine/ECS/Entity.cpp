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
#include "UtilsCPP/Types.hpp"
#include <cassert>

namespace GE
{

Entity::Entity(ECSWorld& world, ECSWorld::EntityID id)
    : m_world(&world), m_entityId(id)
{
}

void Entity::destroy()
{
    if (has<HierarchyComponent>())
    {
        Entity curr = firstChild();
        while (curr)
        {
            curr.parent() = Entity();
            curr = curr.nextChild();
        }
    }

    m_world->deleteEntityID(m_entityId);
    m_world = nullptr;
    m_entityId = INVALID_ENTITY_ID;
}

utils::String& Entity::name()
{
    return get<NameComponent>();
}

Entity& Entity::parent()
{
    return get<HierarchyComponent>().parent;
}

const Entity& Entity::parent() const
{
    return get<HierarchyComponent>().parent;
}

Entity& Entity::firstChild()
{
    return get<HierarchyComponent>().firstChild;
}

const Entity& Entity::firstChild() const
{
    return get<HierarchyComponent>().firstChild;
}

Entity& Entity::nextChild()
{
    return get<HierarchyComponent>().nextChild;
}

const Entity& Entity::nextChild() const
{
    return get<HierarchyComponent>().nextChild;
}

bool Entity::hasParent() const
{
    if (has<HierarchyComponent>() && parent())
        return true;
    return false;
}

utils::uint32 Entity::childCount() const
{
    if (has<HierarchyComponent>() == false)
        return 0;
    utils::uint32 count = 0;
    const Entity* curr = &firstChild();
    while (*curr)
    {
        count++;
        curr = &curr->nextChild();
    }
    return count;
}

Entity Entity::lastChild()
{
    Entity* curr = &firstChild();
    while (*curr && curr->nextChild())
        curr = &curr->nextChild();
    return *curr;
}

const Entity& Entity::lastChild() const
{
    const Entity* curr = &firstChild();
    while (*curr && curr->nextChild())
        curr = &curr->nextChild();
    return *curr;
}

bool Entity::isParentOf(const Entity& entity) const
{
    if (entity.hasParent())
    {
        const Entity* curr = &entity.parent();
        while (*curr)
        {
            if (*curr == *this)
                return true;
            curr = &curr->nextChild();
        }
    }
    return false;    
}

void Entity::addChild(Entity child)
{
    assert(m_world == &child.ecsWorld());
    assert(child.isParentOf(*this) == false);

    if (has<HierarchyComponent>() == false)
        emplace<HierarchyComponent>();
    if (child.has<HierarchyComponent>() == false)
        child.emplace<HierarchyComponent>();

    addChild(child, lastChild());
}

void Entity::addChild(Entity child, Entity after)
{
    assert(m_world == &child.ecsWorld());
    assert(child.isParentOf(*this) == false);

    if (has<HierarchyComponent>() == false)
        emplace<HierarchyComponent>();
    if (child.has<HierarchyComponent>() == false)
        child.emplace<HierarchyComponent>();

    assert(child.parent() == false);
    assert((!after && !firstChild()) || (after && firstChild())); // ensure we not asking to add the child after a certain entity if there is no child

    child.parent() = *this;
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

void Entity::removeParent()
{
    if (hasParent() == false)
        return;
    parent().removeChild(*this);
    parent() = Entity();
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