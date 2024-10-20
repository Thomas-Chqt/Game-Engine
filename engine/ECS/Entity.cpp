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

Entity Entity::parent()
{
    return Entity(*m_world, get<HierarchyComponent>().parent);
}

const Entity Entity::parent() const
{
    return Entity(*m_world, get<HierarchyComponent>().parent);
}

Entity Entity::firstChild()
{
    return Entity(*m_world, get<HierarchyComponent>().firstChild);
}

const Entity Entity::firstChild() const
{
    return Entity(*m_world, get<HierarchyComponent>().firstChild);
}

Entity Entity::nextChild()
{
    return Entity(*m_world, get<HierarchyComponent>().nextChild);
}

const Entity Entity::nextChild() const
{
    return Entity(*m_world, get<HierarchyComponent>().nextChild);
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

    HierarchyComponent& thisComp = this->get<HierarchyComponent>();
    HierarchyComponent& childComp = child.get<HierarchyComponent>();

    assert((!after && !firstChild()) || (after && firstChild())); // ensure we not asking to add the child after a certain entity if there is no child

    childComp.parent = m_entityId;
    if (!after && !firstChild())
    {
        childComp.nextChild = INVALID_ENTITY_ID;
        thisComp.firstChild = child.entityID();
    }
    else
    {
        HierarchyComponent& afterComp = after.get<HierarchyComponent>();
        childComp.nextChild = after.nextChild().entityID();
        afterComp.nextChild = child.entityID();
    }
}

void Entity::removeChild(Entity child)
{
    assert(m_world == &child.ecsWorld());

    HierarchyComponent& thisComp = this->get<HierarchyComponent>();
    HierarchyComponent& childComp = child.get<HierarchyComponent>();

    if (thisComp.firstChild == child.entityID())
        thisComp.firstChild = child.nextChild().entityID();
    else
    {
        Entity curr = firstChild();
        HierarchyComponent& currComp = curr.get<HierarchyComponent>();
        while (currComp.nextChild != child.entityID())
            curr = curr.nextChild();
        currComp.nextChild = curr.nextChild().nextChild().entityID();
    }

    childComp.parent = INVALID_ENTITY_ID;
    childComp.nextChild = INVALID_ENTITY_ID;
}

void Entity::removeParent()
{
    if (hasParent() == false)
        return;
    HierarchyComponent& comp = get<HierarchyComponent>();
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
    if (hasParent())
        return parent().worldTransform() * transform();
    return transform();
}

}