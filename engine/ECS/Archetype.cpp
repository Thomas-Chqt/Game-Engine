/*
 * ---------------------------------------------------
 * Archetype.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/16 16:37:16
 * ---------------------------------------------------
 */

#include "ECS/ECSWorld.hpp"
#include "UtilsCPP/Types.hpp"
#include <cassert>

namespace GE
{

ECSWorld::Archetype::Archetype()
    : m_size(0), m_capacity(0)
{
    addRowType<EntityID>();
}

utils::uint64 ECSWorld::Archetype::allocateCollum()
{
    if (m_size == m_capacity)
        extendCapacity();
    return m_size++;
}

void ECSWorld::Archetype::moveComponents(Archetype& arcSrc, utils::uint64 idxSrc, Archetype& arcDst, utils::uint64 idxDst)
{
    for (auto& [id, row] : arcSrc.m_rows)
    {
        auto it = arcDst.m_rows.find(id);
        if (it != arcDst.m_rows.end())
        {
            row.moveConstructor(
                static_cast<utils::byte*>(row.buffer) + (row.componentSize * idxSrc),
                static_cast<utils::byte*>(it->val.buffer) + (it->val.componentSize * idxDst)
            );
        }
    }
}

void ECSWorld::Archetype::destructCollum(utils::uint64 idx)
{
    for (auto& [_, row] : m_rows)
        row.destructor(static_cast<utils::byte*>(row.buffer) + (row.componentSize * idx));
}

void ECSWorld::Archetype::freeLastCollum()
{
    --m_size;
    if (m_size <= m_capacity / 2)
        reduceCapacity();
}

void ECSWorld::Archetype::setCapacity(utils::uint64 newCapacity)
{
    if (newCapacity == m_capacity)
        return;
    for (auto& [_, row] : m_rows)
    {
        void* newBuffer = operator new (row.componentSize * newCapacity);
        for (utils::uint64 i = 0; i < m_size; i++)
        {
            row.moveConstructor(
                static_cast<utils::byte*>(row.buffer) + (row.componentSize * i),
                static_cast<utils::byte*>(newBuffer) + (row.componentSize * i)
            );
            row.destructor(static_cast<utils::byte*>(row.buffer) + (row.componentSize * i));
        }
        operator delete (row.buffer);
        row.buffer = newBuffer;
    }
    m_capacity = newCapacity;
}

}