/*
 * ---------------------------------------------------
 * ECSWorld.hpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * Date: 2024/07/20 18:41:43
 * ---------------------------------------------------
 */

#ifndef ECSWORLD_HPP
# define ECSWORLD_HPP

#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/Types.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

using EntityID = utils::uint64;

class ECSWorld
{
private:
    using ComponentID = utils::uint32;
    using ArchetypeID = utils::Set<ComponentID>;

    struct Archetype
    {
        ArchetypeID id;
        utils::Dictionary<ComponentID, void*> rows;
        utils::Array<utils::uint32> availableIndices;
        utils::Dictionary<ComponentID, Archetype*> edgeAdd;
        utils::Dictionary<ComponentID, Archetype*> edgeRemove;

        utils::uint32 makeRoom();
    };

    struct EntityData
    {
        Archetype* archetype = nullptr;
        utils::uint32 idx = 0;
    };

public:
    ECSWorld()                = default;
    ECSWorld(const ECSWorld&) = delete;
    ECSWorld(ECSWorld&&)      = delete;
    
    EntityID createEntity();
    void deleteEntity(EntityID);
    
    template<typename T>
    void addComponent(EntityID entityID, const T& component)
    {
        EntityData& entity = m_entityDatas[entityID];

        Archetype* newArchetype = archetypeEdgeAdd(entity.archetype, componentID<T>(), sizeof(T), utils::Func<void(void*)>([](void* ptr){ ((T*)ptr).~T(); }));
        utils::uint32 newArchetypeIdx = newArchetype->makeRoom();
        moveComponents(entity.archetype, entity.idx, newArchetype, newArchetypeIdx);

        new (&newArchetype->rows[componentID<T>()][newArchetypeIdx]) T(component);
    }

    template<typename T>
    void removeComponent(EntityID entityID)
    {
        EntityData& entity = m_entityDatas[entityID];

        ((T*)(entity.archetype->rows[componentID<T>()][entity.idx])).~T();

        Archetype* newArchetype = archetypeEdgeRemove(entity.archetype, componentID<T>());
        utils::uint32 newArchetypeIdx = newArchetype->makeRoom();
        moveComponents(entity.archetype, entity.idx, newArchetype, newArchetypeIdx);
    }

    ~ECSWorld() = default;

private:
    template<typename T> inline ComponentID componentID() { static ComponentID id = m_nextComponentID++; return id; }

    Archetype* archetypeEdgeAdd(Archetype*, ComponentID, utils::uint32 size, const utils::Func<void(void*)>& destructor);
    Archetype* archetypeEdgeRemove(Archetype*, ComponentID);

    void moveComponents(Archetype* src, utils::uint32 srcIdx, Archetype* dst, utils::uint32 dstIdx);

    ComponentID m_nextComponentID = 0;

    utils::Array<EntityData> m_entityDatas;
    utils::Array<EntityID> m_availableEntityIDs;
    
    utils::Dictionary<ArchetypeID, utils::UniquePtr<Archetype>> m_archetypes;
    
public:
    ECSWorld& operator = (const ECSWorld&) = delete;
    ECSWorld& operator = (ECSWorld&&)      = delete;
};

}

#endif // ECSWORLD_HPP