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
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

Entity ECSWorld::createEntity()
{
    Entity newEntity = 0;
    if (m_availableEntities.isEmpty())
    {
        newEntity = m_entityDatas.length();
        m_entityDatas.append(EntityData());
    }
    else
    {
        newEntity = m_availableEntities.first();
        m_entityDatas[newEntity] = EntityData();
        m_availableEntities.remove(m_availableEntities.begin());
    }
    return newEntity;
}

void ECSWorld::deleteEntity(Entity entityIdx)
{
    EntityData& entity = m_entityDatas[entityIdx];
    if (entity.archetype != nullptr)
        entity.archetype->deleteEntry(entity.idx);
    m_availableEntities.append(entityIdx);
}

void ECSWorld::addComponent(Entity entityIdx, ComponentID componentID, void* data, utils::uint32 size, utils::Func<void(void*)> destructor)
{
    EntityData& entity = m_entityDatas[entityIdx];

    Archetype* archetypeDst = nullptr;
    if (entity.archetype == nullptr)
    {
        auto newArcIt = m_rootArchetypes.find(componentID);
        if (newArcIt == m_rootArchetypes.end())
            archetypeDst = m_rootArchetypes.insert(componentID, utils::UniquePtr<Archetype>(new Archetype(componentID, size, destructor)))->val;
        else
            archetypeDst = newArcIt->val;
    }
    else
    {

    }
    ArchetypeEntryIdx archetypeDstIdx = archetypeDst->newEntry();
}

}