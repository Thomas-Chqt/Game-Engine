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
    m_archetypes.insert(ArchetypeID{0}, Archetype());
}

ECSWorld::EntityID ECSWorld::newEntityID()
{
    // new entity has no component so directly inserting in empty archetype (the one with only the entity id)
    ArchetypeID newEntityArcId = { 0 };
    Archetype& newEntityArchetype = m_archetypes[newEntityArcId];
    utils::uint64 newEntityIdx = newEntityArchetype.allocateCollum();

    EntityID newEntityId;
    if (m_availableEntityIDs.isEmpty())
    {
        newEntityId = m_entityDatas.length();
        m_entityDatas.append(EntityData{newEntityArcId, newEntityIdx});
    }
    else
    {
        newEntityId = m_availableEntityIDs.pop(m_availableEntityIDs.begin());
        m_entityDatas[newEntityId] = EntityData{newEntityArcId, newEntityIdx};
    }
    newEntityArchetype.getEntityID(newEntityIdx) = newEntityId;

    assert(isValidEntityID(newEntityId));
    return newEntityId;
}

void ECSWorld::deleteEntityID(EntityID entityId)
{
    assert(isValidEntityID(entityId));
    Archetype& entityArch = m_archetypes[m_entityDatas[entityId].archetypeId];
    utils::uint64 entityIdx = m_entityDatas[entityId].idx;

    // last entity of the archetype will be move to the index of the delete entity
    // so the idx in the entity datas need to be change
    m_entityDatas[entityArch.getEntityID(entityArch.size() - 1)].idx = entityIdx;

    entityArch.destructCollum(entityIdx);
    Archetype::moveComponents(entityArch, entityArch.size() - 1, entityArch, entityIdx);
    entityArch.destructCollum(entityArch.size() - 1);
    entityArch.freeLastCollum();

    m_availableEntityIDs.insert(entityId);
}

utils::uint32 ECSWorld::componentCount()
{
    utils::uint64 count = 0;
    for (auto& [id, arch] : m_archetypes)
        count += (id.size() - 1) * arch.size();
    return count;
}

ECSWorld::ComponentID ECSWorld::nextComponentID()
{
    // start at 1 because 0 is reserved for the entity id
    static ComponentID id = 1;
    return id++;
};

}