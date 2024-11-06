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
    assert(m_world->isValidEntityID(id));
}

void Entity::destroy()
{
    if (has<HierarchyComponent>())
    {
        Entity curr = firstChild();
        while (curr)
        {
            curr.get<HierarchyComponent>().parent = INVALID_ENTITY_ID;
            curr = curr.nextChild();
        }
        if (parent())
            parent().removeChild(*this);
    }

    m_world->deleteEntityID(m_entityId);
    m_world = nullptr;
    m_entityId = INVALID_ENTITY_ID;
}

utils::String& Entity::name()
{
    return get<NameComponent>();
}

Entity Entity::parent()
{
    ECSWorld::EntityID parentId = get<HierarchyComponent>().parent;
    if (parentId == INVALID_ENTITY_ID)
        return Entity();
    return Entity(*m_world, parentId);
}

Entity Entity::firstChild()
{
    ECSWorld::EntityID firstChildId = get<HierarchyComponent>().firstChild;
    if (firstChildId == INVALID_ENTITY_ID)
        return Entity();
    return Entity(*m_world, firstChildId);
}

Entity Entity::nextChild()
{
    ECSWorld::EntityID nextChildId = get<HierarchyComponent>().nextChild;
    if (nextChildId == INVALID_ENTITY_ID)
        return Entity();
    return Entity(*m_world, nextChildId);
}

bool Entity::hasParent() const
{
    if (has<HierarchyComponent>() && parent())
        return true;
    return false;
}

utils::uint32 Entity::childCount() const
{
    utils::uint32 count = 0;
    if (has<HierarchyComponent>())
    {
        Entity curr = firstChild();
        while (curr)
        {
            count++;
            curr = curr.nextChild();
        }
    }
    return count;
}

Entity Entity::lastChild()
{
    Entity curr = firstChild();
    while (curr && curr.nextChild())
        curr = curr.nextChild();
    return curr;
}

const Entity Entity::lastChild() const
{
    Entity curr = firstChild();
    while (curr && curr.nextChild())
        curr = curr.nextChild();
    return curr;
}

bool Entity::isParentOf(const Entity& entity) const
{
    if (entity.hasParent())
    {
        Entity curr = entity.parent();
        while (curr)
        {
            if (curr == *this)
                return true;
            curr = curr.nextChild();
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
    assert(child.hasParent() == false);
    assert(child.isParentOf(*this) == false);

    if (has<HierarchyComponent>() == false)
        emplace<HierarchyComponent>();
    if (child.has<HierarchyComponent>() == false)
        child.emplace<HierarchyComponent>();

    assert((!after && !firstChild()) || (after && firstChild())); // ensure we not asking to add the child after a certain entity if there is no child

    HierarchyComponent& childComp = child.get<HierarchyComponent>();
    childComp.parent = m_entityId;
    if (!after && !firstChild())
    {
        childComp.nextChild = INVALID_ENTITY_ID;
        get<HierarchyComponent>().firstChild = child.entityID();
    }
    else
    {
        childComp.nextChild = after.nextChild().entityID();
        after.get<HierarchyComponent>().nextChild = child.entityID();
    }
}

void Entity::removeChild(Entity child)
{
    assert(m_world == &child.ecsWorld());

    if (firstChild() == child)
        get<HierarchyComponent>().firstChild = firstChild().nextChild().entityID();
    else
    {
        Entity curr = firstChild();
        while (curr.nextChild() != child)
            curr = curr.nextChild();
        curr.get<HierarchyComponent>().nextChild = curr.nextChild().nextChild().entityID();
    }

    child.get<HierarchyComponent>().parent = INVALID_ENTITY_ID;
    child.get<HierarchyComponent>().nextChild = INVALID_ENTITY_ID;
}

void Entity::removeParent()
{
    if (hasParent() == false)
        return;
    parent().removeChild(*this);
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

math::mat4x4 Entity::transform() const
{
    return get<TransformComponent>();
}

math::mat4x4 Entity::worldTransform() const
{
    if (hasParent() && parent().has<TransformComponent>())
        return parent().worldTransform() * transform();
    return transform();
}

}