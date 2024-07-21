/*
 * ---------------------------------------------------
 * Archetype.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/21 11:48:47
 * ---------------------------------------------------
 */

#include "ECS/Archetype.hpp"
#include <cstring>
#include <new>

namespace GE
{

Archetype::Archetype(ECSWorld& world) : m_world(world)
{
}

utils::uint32 Archetype::newEntry()
{
    if (m_availableIndices.isEmpty() == false)
    {
        auto idx = m_availableIndices.first();
        m_availableIndices.remove(m_availableIndices.begin());
        return idx;
    }

    if (m_bufferSize <= m_elementCount)
    {
        for (auto& keyVal: m_rows)
        {
            void* newBuffer = operator new (m_bufferSize * 2);
            std::memcpy(newBuffer, keyVal.val.buffer, m_bufferSize);
            operator delete (keyVal.val.buffer);
            keyVal.val.buffer = newBuffer;
        }
        m_bufferSize *= 2;
    }
    return m_elementCount++;
}

void Archetype::deleteEntry(utils::uint32 idx)
{
}

Archetype* Archetype::edgesAdd(ComponentID compID)
{
    auto it = m_edgesAdd.find(compID);
    if (it != m_edgesAdd.end())
        return it->val;
}

}