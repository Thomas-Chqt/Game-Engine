/*
 * ---------------------------------------------------
 * Archetype.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/06 13:50:59
 * ---------------------------------------------------
 */

#ifndef ARCHETYPE_HPP
#define ARCHETYPE_HPP

#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/Types.hpp"
#include "ECS/ECSWorld.hpp"

namespace GE
{

class Archetype
{
private:
    struct Row
    {
        Archetype& archetype;

        const utils::uint32 elementSize;
        const utils::Func<void(void*)> destructor;

        void* buffer = nullptr;
        
        Row(Archetype&, utils::uint32 elementSize, const utils::Func<void(void*)>& destructor);
        Row(const Row&) = delete;
        Row(Row&&);
        ~Row();
    };

public:
    Archetype()                 = default;
    Archetype(const Archetype&) = delete;
    Archetype(Archetype&&)      = delete;

    inline const ECSWorld::ArchetypeID& id() const { return m_id; }
    inline utils::uint32 entityCount() const { return m_size - m_availableIndices.size(); }
    inline utils::uint32 maxIdx() const { return m_size; }

    utils::uint32 newIndex();
    bool isValidIdx(utils::uint32);

    void* getComponent(ECSWorld::ComponentID, utils::uint32 idx);
    utils::uint64* getEntityId(utils::uint32 idx);

    Archetype*& edgeAdd(ECSWorld::ComponentID);
    Archetype*& edgeRemove(ECSWorld::ComponentID);

    void deleteComponents(utils::uint32 idx);
    void destructComponent(ECSWorld::ComponentID componentID, utils::uint32 idx);

    utils::UniquePtr<Archetype> duplicate() const;
    void addRow(ECSWorld::ComponentID componentID, utils::uint32 elementSize, const utils::Func<void(void*)>& destructor);
    void removeRow(ECSWorld::ComponentID);

    static void moveComponents(Archetype* src, utils::uint32 srcIdx, Archetype* dst, utils::uint32 dstIdx);

    ~Archetype() = default;

private:
    ECSWorld::ArchetypeID m_id;

    utils::uint32 m_size = 0;
    utils::uint32 m_capacity = 1;

    utils::Set<utils::uint32> m_availableIndices;

    utils::Dictionary<ECSWorld::ComponentID, Row> m_rows;
    utils::Array<utils::uint64> m_entityIdsRow;
    
    utils::Dictionary<ECSWorld::ComponentID, Archetype*> m_edgeAdd;
    utils::Dictionary<ECSWorld::ComponentID, Archetype*> m_edgeRemove;

public:
    Archetype& operator = (const Archetype&) = delete;
    Archetype& operator = (Archetype&&)      = delete;
};

}

#endif // ARCHETYPE_HPP