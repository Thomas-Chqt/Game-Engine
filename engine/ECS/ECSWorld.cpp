/*
 * ---------------------------------------------------
 * ECSWorld.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/21 09:35:25
 * ---------------------------------------------------
 */

#include "ECS/ECSWorld.hpp"
#include "UtilsCPP/Types.hpp"
#include <cassert>

namespace GE
{

ECSWorld::ECSWorld()
{
    m_archetypes.insert(ArchetypeID(), Archetype());
}

ECSWorld::EntityID ECSWorld::newEntityID()
{
    ArchetypeID newEntityArcId = ArchetypeID();
    utils::uint64 newEntityIdx = m_archetypes[newEntityArcId].allocateCollum();

    EntityID newEntityId;
    if (m_availableEntityIDs.isEmpty())
    {
        newEntityId = m_entityDatas.length();
        m_entityDatas.append(EntityData{ newEntityArcId, newEntityIdx});
    }
    else
    {
        newEntityId = m_availableEntityIDs.pop(m_availableEntityIDs.begin());
        new (&m_entityDatas[newEntityId]) EntityData();
    }
    assert(isValidEntityID(newEntityId));
    return newEntityId;
}

void ECSWorld::deleteEntityID(EntityID entityId)
{
    assert(isValidEntityID(entityId));
    EntityData& entityData = m_entityDatas[entityId];
    Archetype& entityArch = m_archetypes[entityData.archetypeId];

    entityArch.destructCollum(entityData.idx);
    Archetype::moveComponents(entityArch, entityArch.size() - 1, entityArch, entityData.idx);
    entityArch.freeLastCollum();

    m_availableEntityIDs.insert(entityId);
}

ECSWorld::ComponentID ECSWorld::nextComponentID()
{
    static ComponentID id = 0;
    return id++;
};

}