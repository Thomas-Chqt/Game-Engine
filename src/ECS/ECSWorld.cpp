/*
 * ---------------------------------------------------
 * ECSWorld.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/21 09:35:25
 * ---------------------------------------------------
 */

#include "Game-Engine/ECSWorld.hpp"
#include "ECS/Archetype.hpp"
#include "Game-Engine/Components.hpp"
#include "ECS/InternalComponents.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/Types.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <cstring>
#include "Game-Engine/Entity.hpp"

namespace GE
{

ECSWorld::ECSWorld()
{
    m_emptyArchetype = utils::makeUnique<Archetype>();
}

Entity ECSWorld::newEmptyEntity()
{
    utils::uint32 newEntityArchIdx = m_emptyArchetype->newIndex();

    EntityID newEntityId = 0;
    if (m_availableEntityIDs.isEmpty())
    {
        newEntityId = m_entityDatas.length();
        m_entityDatas.append(EntityData{m_emptyArchetype, newEntityArchIdx});
    }
    else
    {
        newEntityId = m_availableEntityIDs.pop(m_availableEntityIDs.begin());
        new (&m_entityDatas[newEntityId]) EntityData{m_emptyArchetype, newEntityArchIdx};
    }
    *m_emptyArchetype->getEntityId(newEntityArchIdx) = newEntityId;
    return Entity(*this, newEntityId);
}

Entity ECSWorld::newEntity(const utils::String& name)
{
    Entity newEntity = newEmptyEntity();

    newEntity.emplace<NameComponent>(name);
    newEntity.emplace<TransformComponent>(
        math::vec3f{ 0.0F, 0.0F, 0.0F }, // position
        math::vec3f{ 0.0F, 0.0F, 0.0F }, // rotation
        math::vec3f{ 1.0F, 1.0F, 1.0F }  // scale
    );

    return newEntity;
}

Entity ECSWorld::makeEntityScriptable(utils::UniquePtr<Entity>&& entity)
{
    return *entity->emplace<ScriptComponent>(std::move(entity)).instance;
}

utils::uint32 ECSWorld::componentCount()
{
    utils::uint32 output = 0;
    for (auto& [id, arc] : m_archetypes) {
        output += id.size() * arc->entityCount();
    }
    return output;
}

ECSWorld::~ECSWorld()
{
}


void ECSWorld::deleteEntity(EntityID entityId)
{
    EntityData& entityData = m_entityDatas[entityId];
    entityData.archetype->deleteComponents(entityData.idx);
    m_availableEntityIDs.insert(entityId);
}


void* ECSWorld::emplace(EntityID entityId, ComponentID componentID, utils::uint32 size, const utils::Func<void(void*)>& constructor, const utils::Func<void(void*)>& destructor)
{
    EntityData& entityData = m_entityDatas[entityId];
    Archetype*& newArchetype = entityData.archetype->edgeAdd(componentID);
    if (newArchetype == nullptr)
    {
        ArchetypeID newArchetypeId = entityData.archetype->id() + componentID;
        utils::Dictionary<ArchetypeID, utils::UniquePtr<Archetype>>::Iterator it = m_archetypes.find(newArchetypeId);
        if (it == m_archetypes.end())
        {
            it = m_archetypes.insert(newArchetypeId, entityData.archetype->duplicate());
            it->val->addRow(componentID, size, destructor);
        }
        newArchetype = it->val;
    }
    utils::uint32 newArchetypeIdx = newArchetype->newIndex();
    Archetype::moveComponents(entityData.archetype, entityData.idx, newArchetype, newArchetypeIdx);
    entityData.archetype = newArchetype;
    entityData.idx = newArchetypeIdx;

    void* componentPtr = entityData.archetype->getComponent(componentID, entityData.idx);
    constructor(componentPtr);

    return componentPtr;
}

void ECSWorld::remove(EntityID entityId, ComponentID componentID)
{
    EntityData& entityData = m_entityDatas[entityId];

    entityData.archetype->destructComponent(componentID, entityData.idx);

    Archetype*& newArchetype = entityData.archetype->edgeRemove(componentID);
    if (newArchetype == nullptr)
    {
        ArchetypeID newArchetypeId = entityData.archetype->id() - componentID;
        if (newArchetypeId.isEmpty())
            newArchetype = m_emptyArchetype;
        else
        {
            utils::Dictionary<ArchetypeID, utils::UniquePtr<Archetype>>::Iterator it = m_archetypes.find(newArchetypeId);
            if (it == m_archetypes.end())
            {
                it = m_archetypes.insert(newArchetypeId, entityData.archetype->duplicate());
                it->val->removeRow(componentID);
            }
            newArchetype = it->val;
        }
    }
    utils::uint32 newArchetypeIdx = newArchetype->newIndex();
    Archetype::moveComponents(entityData.archetype, entityData.idx, newArchetype, newArchetypeIdx);
    entityData.archetype = newArchetype;
    entityData.idx = newArchetypeIdx;
}

bool ECSWorld::has(EntityID entityId, ComponentID componentID)
{
    EntityData& entityData = m_entityDatas[entityId];
    return entityData.archetype->id().contain(componentID);
}

void* ECSWorld::get(EntityID entityId, ComponentID componentID)
{
    EntityData& entityData = m_entityDatas[entityId];
    return entityData.archetype->getComponent(componentID, entityData.idx);
}

}