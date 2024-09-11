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
#include "Math/Vector.hpp"
#include <cassert>
#include <utility>

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
    if (has<HierarchicalComponent>() == false)
        emplace<HierarchicalComponent>();

    if (child.has<HierarchicalComponent>() == false)
        child.emplace<HierarchicalComponent>();
    
    HierarchicalComponent& thisComp = this->get<HierarchicalComponent>();
    HierarchicalComponent& childComp = child.get<HierarchicalComponent>();

    assert(childComp.parent == false);
    childComp.parent = *this;
    childComp.nextChild = thisComp.firstChild;
    thisComp.firstChild = child;
}

Entity Entity::popChild()
{
    assert(has<HierarchicalComponent>());

    HierarchicalComponent& thisComp = this->get<HierarchicalComponent>();

    Entity prevFirstChild = thisComp.firstChild;

    HierarchicalComponent& prevFirstChildComp = prevFirstChild.get<HierarchicalComponent>();

    thisComp.firstChild = prevFirstChildComp.nextChild;
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

void Entity::setScriptInstance(utils::UniquePtr<Entity>&& instance)
{
    ScriptComponent* comp;
    if (has<ScriptComponent>())
        comp = &get<ScriptComponent>();
    else
        comp = &emplace<ScriptComponent>();
    comp->instance = std::move(instance);
}

}