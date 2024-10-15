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
#include "UtilsCPP/UniquePtr.hpp"
#include <cassert>

namespace GE
{

ECSWorld::EntityID ECSWorld::newEntityID()
{
    EntityID newEntityId = 0;
    if (m_availableEntityIDs.isEmpty())
    {
        newEntityId = m_entityDatas.length();
        m_entityDatas.append(EntityData());
    }
    else
    {
        newEntityId = m_availableEntityIDs.pop(m_availableEntityIDs.begin());
        new (&m_entityDatas[newEntityId]) EntityData();
    }
    return newEntityId;
}

void ECSWorld::deleteEntityID(EntityID entityId)
{
    EntityData& entityData = m_entityDatas[entityId];
    if (entityData.archetype)
    {
        destructComponents(*entityData.archetype, entityData.idx);
        freeComponents(*entityData.archetype, entityData.idx);
    }
    m_availableEntityIDs.insert(entityId);
}

void ECSWorld::moveComponents(Archetype& arcSrc, utils::uint64 idxSrc, Archetype& arcDst, utils::uint64 idxDst)
{
    for (auto& [id, row] : arcSrc.rows)
    {
        auto it = arcDst.rows.find(id);
        if (it != arcDst.rows.end())
            row.componentMoveConstructor(
                &((utils::byte*)row.buffer)[row.componentSize * idxSrc],
                &((utils::byte*)it->val.buffer)[it->val.componentSize * idxDst]
            );
    }
    assert(isValidEntityID(arcSrc.entityIdsRow[idxSrc]));
    m_entityDatas[arcSrc.entityIdsRow[idxSrc]].archetype = &arcDst;
    m_entityDatas[arcSrc.entityIdsRow[idxSrc]].idx = idxDst;
    arcDst.entityIdsRow[idxDst] = arcSrc.entityIdsRow[idxSrc];
}

void ECSWorld::destructComponents(Archetype& archetype, utils::uint64 idx)
{
    for (auto& [id, row] : archetype.rows)
        row.componentDestructor(&((utils::byte*)row.buffer)[row.componentSize * idx]);
    assert(isValidEntityID(archetype.entityIdsRow[idx]));
    m_entityDatas[archetype.entityIdsRow[idx]].archetype = nullptr;
    m_entityDatas[archetype.entityIdsRow[idx]].idx = 0;
}

void ECSWorld::freeComponents(Archetype& archetype, utils::uint64 idx)
{
    assert(isValidEntityID(archetype.entityIdsRow.last()));
    m_entityDatas[archetype.entityIdsRow.last()].idx = idx;
    moveComponents(archetype, archetype.entityIdsRow.length() - 1, archetype, idx);
    archetype.entityIdsRow[idx] = archetype.entityIdsRow.last();
    utils::uint64 prevCap = archetype.entityIdsRow.capacity();
    archetype.entityIdsRow.remove(--archetype.entityIdsRow.end());
    if (archetype.entityIdsRow.capacity() != prevCap)
        changeArchetypeRowCapacity(archetype, archetype.entityIdsRow.capacity());
}

void ECSWorld::changeArchetypeRowCapacity(Archetype& archetype, utils::uint64 cap)
{
    for (auto& [_, row] : archetype.rows)
    {
        utils::byte* newBuffer = (utils::byte*)operator new (cap);
        for (utils::uint64 i = 0; i < archetype.entityIdsRow.length(); i++)
        {
            row.componentMoveConstructor(
                &((utils::byte*)row.buffer)[row.componentSize * i],
                &((utils::byte*)newBuffer)[row.componentSize * i]
            );
            row.componentDestructor(&((utils::byte*)row.buffer)[row.componentSize * i]);
        }
        operator delete (row.buffer);
        row.buffer = newBuffer;
    }
}

ECSWorld::Archetype& ECSWorld::findOrCreateArchetype(const ArchetypeID& id)
{
    auto it = m_archetypes.find(id);
    if (it == m_archetypes.end())
        it = m_archetypes.insert(utils::makeUnique<Archetype>(id));
    return **it;
}

}