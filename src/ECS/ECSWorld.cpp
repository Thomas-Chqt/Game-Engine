/*
 * ---------------------------------------------------
 * ECSWorld.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/21 09:35:25
 * ---------------------------------------------------
 */

#include "Game-Engine/ECSWorld.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/Types.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <cstring>

namespace GE
{

ECSWorld::Archetype::Row::~Row()
{
    for (utils::uint32 i = 0; i < archetype.entryCount; i++)
    {
        if (archetype.availableIndices.contain(i) == false)
            destructor(buffer + (elementSize * i));
    }
    operator delete (buffer);
}

utils::uint32 ECSWorld::componentCount() const
{
    utils::uint32 output = 0;
    for (auto& [_, archetype] : m_archetypes)
        output += archetype->rows.size() * (archetype->entryCount - archetype->availableIndices.size());
    return output;
}

EntityID ECSWorld::createEntity()
{
    EntityID newEntity = 0;
    if (m_availableEntityIDs.isEmpty())
    {
        newEntity = m_entityDatas.length();
        m_entityDatas.append(EntityData());
    }
    else
    {
        newEntity = m_availableEntityIDs.pop(m_availableEntityIDs.begin());
        new (&m_entityDatas[newEntity]) EntityData;
    }
    return newEntity;
}

void ECSWorld::deleteEntity(EntityID entityIdx)
{
    EntityData& entity = m_entityDatas[entityIdx];
    if (entity.archetype != nullptr)
    {
        for (const auto& [_, row] : entity.archetype->rows)
            row.destructor(row.buffer + (row.elementSize * entity.idx));
        entity.archetype->availableIndices.insert(entity.idx);
    }
    m_availableEntityIDs.insert(entityIdx);
}

ECSWorld::Archetype* ECSWorld::archetypeEdgeAdd(Archetype* archetype, ComponentID componentID, utils::uint32 size, const utils::Func<void(void*)>& destructor)
{
    if (archetype == nullptr)
    {
        ArchetypeID outputArcID = { componentID };
        utils::Dictionary<ArchetypeID, utils::UniquePtr<Archetype>>::Iterator it = m_archetypes.find(outputArcID);
        if (it == m_archetypes.end())
        {
            it = m_archetypes.insert(outputArcID, utils::makeUnique<Archetype>(outputArcID));
            it->val->rows.insert(componentID, Archetype::Row(*it->val, size, destructor));
        }
        return (Archetype*)it->val;
    }
    else
    {
        utils::Dictionary<ComponentID, Archetype*>::Iterator edgeIt = archetype->edgeAdd.find(componentID);
        if (edgeIt == archetype->edgeAdd.end())
        {
            ArchetypeID outputArcID = archetype->id + componentID;
            utils::Dictionary<ArchetypeID, utils::UniquePtr<Archetype>>::Iterator arcIt = m_archetypes.find(outputArcID);
            if (arcIt == m_archetypes.end())
            {
                arcIt = m_archetypes.insert(outputArcID, utils::makeUnique<Archetype>(outputArcID));
                for (auto& [compId, row] : archetype->rows)
                    arcIt->val->rows.insert(compId, Archetype::Row(*arcIt->val, row.elementSize, row.destructor));
                arcIt->val->rows.insert(componentID, Archetype::Row(*arcIt->val, size, destructor));
            }
            edgeIt = archetype->edgeAdd.insert(componentID, (Archetype*)arcIt->val);
        }
        return edgeIt->val;
    }
}

ECSWorld::Archetype* ECSWorld::archetypeEdgeRemove(Archetype* archetype, ComponentID componentID)
{
    ArchetypeID outputArcID = archetype->id - componentID;
    if (outputArcID.isEmpty())
        return nullptr;
    utils::Dictionary<ComponentID, Archetype*>::Iterator edgeIt = archetype->edgeRemove.find(componentID);
    if (edgeIt == archetype->edgeRemove.end())
    {
        utils::Dictionary<ArchetypeID, utils::UniquePtr<Archetype>>::Iterator arcIt = m_archetypes.find(outputArcID);
        if (arcIt == m_archetypes.end())
        {
            arcIt = m_archetypes.insert(outputArcID, utils::makeUnique<Archetype>(outputArcID));
            for (auto& compId : outputArcID)
                arcIt->val->rows.insert(compId, Archetype::Row(*arcIt->val, archetype->rows[compId].elementSize, archetype->rows[compId].destructor));
        }
        edgeIt = archetype->edgeRemove.insert(componentID, (Archetype*)arcIt->val);
    }
    return edgeIt->val;
}

utils::uint32 ECSWorld::nextAvailableIdx(Archetype* archetype)
{
    if (archetype == nullptr)
        return 0;
    if (archetype->availableIndices.isEmpty() == false)
        return archetype->availableIndices.pop(archetype->availableIndices.begin());
    if (archetype->entryCapacity == archetype->entryCount)
    {
        archetype->entryCapacity = archetype->entryCapacity == 0 ? 1 : archetype->entryCapacity * 2;
        for (auto& [_, row] : archetype->rows)
        {
            utils::byte* newBuffer = (utils::byte*)operator new (archetype->entryCapacity * row.elementSize);
            std::memcpy(newBuffer, row.buffer, archetype->entryCount * row.elementSize);
            operator delete (row.buffer);
            row.buffer = newBuffer;
        }
    }
    return archetype->entryCount++;
}

void ECSWorld::moveComponents(Archetype* src, utils::uint32 srcIdx, Archetype* dst, utils::uint32 dstIdx)
{
    if (src == nullptr)
        return;
    for (auto& [compID, row] : src->rows)
    {
        if (dst != nullptr && dst->rows.contain(compID))
        {
            Archetype::Row& dstRow = dst->rows[compID];
            std::memcpy(
                dstRow.buffer + dstRow.elementSize * dstIdx,
                row.buffer    + row.elementSize    * srcIdx,
                row.elementSize
            );
        }
    }
    src->availableIndices.insert(srcIdx);
}

}