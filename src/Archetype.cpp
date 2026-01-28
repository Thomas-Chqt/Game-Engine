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
    : m_size(0), m_capacity(1)
{
    m_rows.insert(0, Row{
        operator new (sizeof(EntityID) * m_capacity),
        componentSize<EntityID>(),
        componentCopyConstructor<EntityID>(),
        componentMoveConstructor<EntityID>(),
        componentDestructor<EntityID>(),
    });
}

ECSWorld::Archetype::Archetype(const Archetype& cp)
    : m_size(cp.m_size), m_capacity(cp.m_capacity)
{
    for (auto& [id, row] : cp.m_rows)
    {
        Row& newRow = m_rows.insert(id, Row{
            operator new (row.componentSize * m_capacity),
            row.componentSize, row.copyConstructor,
            row.moveConstructor, row.destructor
        })->val;

        for (utils::uint64 idx = 0; idx < cp.m_size; idx++)
        {
            row.copyConstructor(
                static_cast<utils::byte*>(row.buffer) + (row.componentSize * idx),
                static_cast<utils::byte*>(newRow.buffer) + (newRow.componentSize * idx)
            );
        }
    }
}

ECSWorld::Archetype::Archetype(Archetype&& mv)
    : m_size(mv.m_size), m_capacity(mv.m_capacity)
{
    for (auto& [id, row] : mv.m_rows)
    {
        Row& newRow = m_rows.insert(id, Row{
            row.buffer, row.componentSize, row.copyConstructor,
            row.moveConstructor, row.destructor
        })->val;
        row.buffer = nullptr;
    }
}

ECSWorld::Archetype ECSWorld::Archetype::duplicateRowTypes()
{
    Archetype newArchetype;
    // remove the entity id row because it will be copied
    for (auto& [id, row] : newArchetype.m_rows)
    {
        if (row.buffer != nullptr)
        {
            for (utils::uint64 i = 0; i < newArchetype.m_size; i++)
                row.destructor(static_cast<utils::byte*>(row.buffer) + (row.componentSize * i));
            operator delete (row.buffer);
        }
    }
    newArchetype.m_rows.clear();
    for (auto& [id, row] : m_rows)
    {
        newArchetype.m_rows.insert(id, Row{
            operator new (row.componentSize * newArchetype.m_capacity),
            row.componentSize, row.copyConstructor,
            row.moveConstructor, row.destructor
        });
    }
    return newArchetype;
}

utils::uint64 ECSWorld::Archetype::allocateCollum()
{
    if (m_size == m_capacity)
        extendCapacity();
    return m_size++;
}

ECSWorld::EntityID& ECSWorld::Archetype::getEntityID(utils::uint64 idx)
{
    return static_cast<EntityID*>(m_rows[0].buffer)[idx];
}

const ECSWorld::EntityID& ECSWorld::Archetype::getEntityID(utils::uint64 idx) const
{
    return static_cast<const EntityID*>(m_rows[0].buffer)[idx];
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

ECSWorld::Archetype::~Archetype()
{
    for (auto& [_, row] : m_rows)
    {
        if (row.buffer != nullptr)
        {
            for (utils::uint64 i = 0; i < m_size; i++)
                row.destructor(static_cast<utils::byte*>(row.buffer) + (row.componentSize * i));
            operator delete (row.buffer);
        }
    }
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

ECSWorld::Archetype& ECSWorld::Archetype::operator = (const Archetype& cp)
{
    if (this != &cp)
    {
        for (auto& [id, row] : m_rows)
        {
            if (row.buffer != nullptr)
            {
                for (utils::uint64 i = 0; i < m_size; i++)
                    row.destructor(static_cast<utils::byte*>(row.buffer) + (row.componentSize * i));
                operator delete (row.buffer);
            }
        }
        m_rows.clear();
        m_size = cp.m_size;
        m_capacity = cp.m_capacity;
        for (auto& [id, row] : cp.m_rows)
        {
            Row& newRow = m_rows.insert(id, Row{
                operator new (row.componentSize * m_capacity),
                row.componentSize, row.copyConstructor,
                row.moveConstructor, row.destructor
            })->val;

            for (utils::uint64 idx = 0; idx < cp.m_size; idx++)
            {
                row.copyConstructor(
                    static_cast<utils::byte*>(row.buffer) + (row.componentSize * idx),
                    static_cast<utils::byte*>(newRow.buffer) + (newRow.componentSize * idx)
                );
            }
        }
    }
    return *this;
}

ECSWorld::Archetype& ECSWorld::Archetype::operator = (Archetype&& mv)
{
    if (this != &mv)
    {
        for (auto& [id, row] : m_rows)
        {
            if (row.buffer != nullptr)
            {
                for (utils::uint64 i = 0; i < m_size; i++)
                    row.destructor(static_cast<utils::byte*>(row.buffer) + (row.componentSize * i));
                operator delete (row.buffer);
            }
        }
        m_rows.clear();
        m_size = mv.m_size;
        m_capacity = mv.m_capacity;
        for (auto& [id, row] : mv.m_rows)
        {
            Row& newRow = m_rows.insert(id, Row{
                row.buffer, row.componentSize, row.copyConstructor,
                row.moveConstructor, row.destructor
            })->val;
            row.buffer = nullptr;
        }
    }
    return *this;
}

}