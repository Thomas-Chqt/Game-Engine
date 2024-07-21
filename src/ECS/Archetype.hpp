/*
 * ---------------------------------------------------
 * Archetype.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/20 19:00:39
 * ---------------------------------------------------
 */

#ifndef ARCHETYPE_HPP
# define ARCHETYPE_HPP

#include "Game-Engine/ECSWorld.hpp"
#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/Types.hpp"

namespace GE
{

class Archetype
{
public:
    Archetype()                 = delete;
    Archetype(const Archetype&) = delete;
    Archetype(Archetype&&)      = delete;

    Archetype(ECSWorld&);

    void addRow(ComponentID, utils::uint32 elementSize, utils::Func<void(void*)> elementDestructor);
    
    utils::uint32 newEntry();
    void deleteEntry(utils::uint32);

    Archetype* edgesAdd(ComponentID);

    ~Archetype() = default;

private:
    struct Row
    {
        utils::uint32 elementSize = 0;
        utils::Func<void(void*)> elementDestructor;

        void* buffer = nullptr;
    };

    ECSWorld& m_world;

    utils::uint32 m_elementCount = 0;
    utils::uint32 m_bufferSize = 0;
    utils::Dictionary<ComponentID, Row> m_rows;
    utils::Array<utils::uint32> m_availableIndices;

    utils::Dictionary<ComponentID, Archetype*> m_edgesAdd;
    utils::Dictionary<ComponentID, Archetype*> m_edgesRmv;
    
public:
    Archetype& operator = (const Archetype&) = delete;
    Archetype& operator = (Archetype&&)      = delete;
};

}

#endif // ARCHETYPE_HPP