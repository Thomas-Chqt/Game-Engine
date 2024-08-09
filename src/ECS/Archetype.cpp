/*
 * ---------------------------------------------------
 * Archetype.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/06 15:51:35
 * ---------------------------------------------------
 */

#include "Archetype.hpp"
#include "UtilsCPP/Types.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <cassert>
#include <cstring>

namespace GE
{

Archetype::Row::Row(Archetype& archetype, utils::uint32 elementSize, const utils::Func<void(void*)>& destructor)
    : archetype(archetype), elementSize(elementSize), destructor(destructor), buffer(nullptr)
{
    if (archetype.m_capacity > 0)
        buffer = operator new (archetype.m_capacity * elementSize);
}

Archetype::Row::Row(Row&& mv)
    : archetype(mv.archetype), elementSize(mv.elementSize),
      destructor(mv.destructor), buffer(mv.buffer)
{
    mv.buffer = nullptr;
}

Archetype::Row::~Row()
{
    if (buffer == nullptr)
        return;
    for (utils::uint32 i = 0; i < archetype.m_size; i++)
    {
        if (archetype.m_availableIndices.contain(i) == false)
            destructor(static_cast<utils::byte*>(buffer) + i * elementSize);
    }
    operator delete (buffer);
}

utils::uint32 Archetype::newIndex()
{
    if (m_availableIndices.isEmpty() == false)
        return m_availableIndices.pop(m_availableIndices.begin());
    if (m_capacity == m_size)
    {
        m_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
        for (auto& [_, row] : m_rows)
        {
            void* newBuffer = operator new (m_capacity * row.elementSize);
            if (m_size > 0)
                std::memcpy(newBuffer, row.buffer, m_size * row.elementSize);
            operator delete (row.buffer);
            row.buffer = newBuffer;
        }
    }
    m_entityIdsRow.append(0);
    return m_size++;
}

bool Archetype::isValidIdx(utils::uint32 idx)
{
    return idx < m_size && m_availableIndices.contain(idx) == false;
}

void* Archetype::getComponent(ECSWorld::ComponentID componentID, utils::uint32 idx)
{
    assert(isValidIdx(idx));
    Row& row = m_rows[componentID];
    return static_cast<utils::byte*>(row.buffer) + idx * row.elementSize;
}

utils::uint64* Archetype::getEntityId(utils::uint32 idx)
{
    assert(isValidIdx(idx));
    return &m_entityIdsRow[idx];
}

Archetype*& Archetype::edgeAdd(ECSWorld::ComponentID componentID)
{
    utils::Dictionary<ECSWorld::ComponentID, Archetype*>::Iterator edgeIt = m_edgeAdd.find(componentID);
    if (edgeIt == m_edgeAdd.end())
        edgeIt = m_edgeAdd.insert(componentID, nullptr);
    return edgeIt->val;
}

Archetype*& Archetype::edgeRemove(ECSWorld::ComponentID componentID)
{
    utils::Dictionary<ECSWorld::ComponentID, Archetype*>::Iterator edgeIt = m_edgeRemove.find(componentID);
    if (edgeIt == m_edgeRemove.end())
        edgeIt = m_edgeRemove.insert(componentID, nullptr);
    return edgeIt->val;

}

void Archetype::deleteComponents(utils::uint32 idx)
{
    for (auto& [_, row] : m_rows)
        row.destructor(static_cast<utils::byte*>(row.buffer) + (idx * row.elementSize));
    m_availableIndices.insert(idx);
}

void Archetype::destructComponent(ECSWorld::ComponentID componentID, utils::uint32 idx)
{
    Row& row = m_rows[componentID];
    row.destructor(static_cast<utils::byte*>(row.buffer) + idx * row.elementSize);
}

utils::UniquePtr<Archetype> Archetype::duplicate() const
{
    utils::UniquePtr<Archetype> newArchetype = utils::makeUnique<Archetype>();
    for (auto& [compId, row] : m_rows)
        newArchetype->addRow(compId, row.elementSize, row.destructor);
    return newArchetype;
}

void Archetype::addRow(ECSWorld::ComponentID componentID, utils::uint32 elementSize, const utils::Func<void(void*)>& destructor)
{
    m_id.insert(componentID);
    m_rows.insert(componentID, Row(*this, elementSize, destructor));
}

void Archetype::removeRow(ECSWorld::ComponentID componentID)
{
    m_id.remove(m_id.find(componentID));
    m_rows.remove(componentID);
}

void Archetype::moveComponents(Archetype* src, utils::uint32 srcIdx, Archetype* dst, utils::uint32 dstIdx)
{
    for (auto& [compID, srcRow] : src->m_rows)
    {
        if (dst != nullptr && dst->m_rows.contain(compID))
        {
            Archetype::Row& dstRow = dst->m_rows[compID];
            std::memcpy(
                static_cast<utils::byte*>(dstRow.buffer) + dstRow.elementSize * dstIdx,
                static_cast<utils::byte*>(srcRow.buffer) + srcRow.elementSize * srcIdx,
                srcRow.elementSize
            );
        }
    }
    dst->m_entityIdsRow[dstIdx] = src->m_entityIdsRow[srcIdx];
    src->m_availableIndices.insert(srcIdx);
}


}