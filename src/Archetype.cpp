/*
 * ---------------------------------------------------
 * Archetype.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/16 16:37:16
 * ---------------------------------------------------
 */

#include "Game-Engine/ECSWorld.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace GE
{

ECSWorld::Archetype::Archetype() : m_size(0), m_capacity(1) // TODO : start capacity at 0 ?
{
    m_rows.insert(std::pair(0, Row{
        operator new (sizeof(EntityID) * m_capacity),
        componentSize<EntityID>(),
        componentCopyConstructor<EntityID>(),
        componentMoveConstructor<EntityID>(),
        componentDestructor<EntityID>(),
    }));
}

ECSWorld::Archetype::Archetype(const Archetype& cp) : m_size(cp.m_size), m_capacity(cp.m_capacity)
{
    for (auto& [id, row] : cp.m_rows)
    {
        auto [it, sucess] = m_rows.insert(std::pair(id, Row{
            operator new (row.componentSize * m_capacity),
            row.componentSize, row.copyConstructor,
            row.moveConstructor, row.destructor
        }));
        assert(sucess);
        Row& newRow = it->second;

        for (uint64_t idx = 0; idx < cp.m_size; idx++)
        {
            row.copyConstructor(
                static_cast<std::byte*>(row.buffer) + (row.componentSize * idx),
                static_cast<std::byte*>(newRow.buffer) + (newRow.componentSize * idx)
            );
        }
    }
}

ECSWorld::Archetype::Archetype(Archetype&& mv)
    : m_size(mv.m_size), m_capacity(mv.m_capacity)
{
    for (auto& [id, row] : mv.m_rows)
    {
        auto [_, sucess] = m_rows.insert(std::pair(id, Row{
            row.buffer, row.componentSize, row.copyConstructor,
            row.moveConstructor, row.destructor
        }));
        assert(sucess);
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
            for (uint64_t i = 0; i < newArchetype.m_size; i++)
                row.destructor(static_cast<std::byte*>(row.buffer) + (row.componentSize * i));
            operator delete (row.buffer);
        }
    }
    newArchetype.m_rows.clear();
    for (auto& [id, row] : m_rows)
    {
        newArchetype.m_rows.insert(std::make_pair(id, Row{
            operator new (row.componentSize * newArchetype.m_capacity),
            row.componentSize, row.copyConstructor,
            row.moveConstructor, row.destructor
        }));
    }
    return newArchetype;
}

uint64_t ECSWorld::Archetype::allocateCollum()
{
    if (m_size == m_capacity)
        extendCapacity();
    return m_size++;
}

void ECSWorld::Archetype::moveComponents(Archetype& arcSrc, uint64_t idxSrc, Archetype& arcDst, uint64_t idxDst)
{
    for (auto& [id, row] : arcSrc.m_rows)
    {
        auto it = arcDst.m_rows.find(id);
        if (it != arcDst.m_rows.end())
        {
            row.moveConstructor(
                static_cast<std::byte*>(row.buffer) + (row.componentSize * idxSrc),
                static_cast<std::byte*>(it->second.buffer) + (it->second.componentSize * idxDst)
            );
        }
    }
}

void ECSWorld::Archetype::destructCollum(uint64_t idx)
{
    for (auto& [_, row] : m_rows)
        row.destructor(static_cast<std::byte*>(row.buffer) + (row.componentSize * idx));
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
            for (uint64_t i = 0; i < m_size; i++)
                row.destructor(static_cast<std::byte*>(row.buffer) + (row.componentSize * i));
            operator delete (row.buffer);
        }
    }
}

void ECSWorld::Archetype::setCapacity(uint64_t newCapacity)
{
    if (newCapacity == m_capacity)
        return;
    for (auto& [_, row] : m_rows)
    {
        void* newBuffer = operator new (row.componentSize * newCapacity);
        for (uint64_t i = 0; i < m_size; i++)
        {
            row.moveConstructor(
                static_cast<std::byte*>(row.buffer) + (row.componentSize * i),
                static_cast<std::byte*>(newBuffer) + (row.componentSize * i)
            );
            row.destructor(static_cast<std::byte*>(row.buffer) + (row.componentSize * i));
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
                for (uint64_t i = 0; i < m_size; i++)
                    row.destructor(static_cast<std::byte*>(row.buffer) + (row.componentSize * i));
                operator delete (row.buffer);
            }
        }
        m_rows.clear();
        m_size = cp.m_size;
        m_capacity = cp.m_capacity;
        for (auto& [id, row] : cp.m_rows)
        {
            auto [it, success] = m_rows.insert(std::make_pair(id, Row{
                operator new (row.componentSize * m_capacity),
                row.componentSize, row.copyConstructor,
                row.moveConstructor, row.destructor
            }));
            assert(success);
            Row& newRow = it->second;

            for (uint64_t idx = 0; idx < cp.m_size; idx++)
            {
                row.copyConstructor(
                    static_cast<std::byte*>(row.buffer) + (row.componentSize * idx),
                    static_cast<std::byte*>(newRow.buffer) + (newRow.componentSize * idx)
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
                for (uint64_t i = 0; i < m_size; i++)
                    row.destructor(static_cast<std::byte*>(row.buffer) + (row.componentSize * i));
                operator delete (row.buffer);
            }
        }
        m_rows.clear();
        m_size = mv.m_size;
        m_capacity = mv.m_capacity;
        for (auto& [id, row] : mv.m_rows)
        {
            auto [_, success] = m_rows.insert(std::make_pair(id, Row{
                row.buffer, row.componentSize, row.copyConstructor,
                row.moveConstructor, row.destructor
            }));
            assert(success);
            row.buffer = nullptr;
        }
    }
    return *this;
}

}
