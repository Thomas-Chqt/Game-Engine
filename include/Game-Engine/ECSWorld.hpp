/*
 * ---------------------------------------------------
 * ECSWorld.hpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * Date: 2024/07/20 18:41:43
 * ---------------------------------------------------
 */

#ifndef ECSWORLD_HPP
#define ECSWORLD_HPP

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
    template<typename ... Ts> friend class ECSWorldView;

    using ComponentID = utils::uint32;
    using ArchetypeID = utils::Set<ComponentID>;

    struct Archetype
    {
        struct Row
        {
            Archetype& archetype;

            utils::uint32 elementSize;
            utils::Func<void(void*)> destructor;

            utils::byte* buffer = nullptr;
            
            inline Row(Archetype& archetype, utils::uint32 elementSize, utils::Func<void(void*)> destructor)
                : archetype(archetype), elementSize(elementSize), destructor(std::move(destructor)) {}   

            ~Row();
        };

        ArchetypeID id;

        utils::uint32 entryCount = 0;
        utils::uint32 entryCapacity = 0;
        utils::Set<utils::uint32> availableIndices;
        utils::Dictionary<ComponentID, Row> rows;
        
        utils::Dictionary<ComponentID, Archetype*> edgeAdd;
        utils::Dictionary<ComponentID, Archetype*> edgeRemove;

        inline Archetype(ArchetypeID id) : id(std::move(id)) {}
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

    inline utils::uint32 entityCount() const { return m_entityDatas.length() - m_availableEntityIDs.size(); }
    inline utils::uint32 archetypeCount() const { return m_archetypes.size(); }
    utils::uint32 componentCount() const;
    
    EntityID createEntity();
    void deleteEntity(EntityID);
    
    template<typename T>
    void addComponent(EntityID entityID, const T& component)
    {
        EntityData& entity = m_entityDatas[entityID];

        Archetype* newArchetype = archetypeEdgeAdd(entity.archetype, componentID<T>(), sizeof(T), utils::Func<void(void*)>([](void* ptr){ ((T*)ptr)->~T(); }));
        utils::uint32 newArchetypeIdx = nextAvailableIdx(newArchetype);
        moveComponents(entity.archetype, entity.idx, newArchetype, newArchetypeIdx);

        new (&((T*)newArchetype->rows[componentID<T>()].buffer)[newArchetypeIdx]) T(component);

        entity.archetype = newArchetype;
        entity.idx = newArchetypeIdx;
    }

    template<typename T>
    void removeComponent(EntityID entityID)
    {
        EntityData& entity = m_entityDatas[entityID];

        ((T*)entity.archetype->rows[componentID<T>()].buffer)[entity.idx].~T();

        Archetype* newArchetype = archetypeEdgeRemove(entity.archetype, componentID<T>());
        utils::uint32 newArchetypeIdx = nextAvailableIdx(newArchetype);
        moveComponents(entity.archetype, entity.idx, newArchetype, newArchetypeIdx);

        entity.archetype = newArchetype;
        entity.idx = newArchetypeIdx;
    }

    template<typename T>
    T& getComponent(EntityID entityID)
    {
        EntityData& entityData = m_entityDatas[entityID];
        Archetype::Row& row = entityData.archetype->rows[componentID<T>()];
        return ((T*)row.buffer)[entityData.idx];
    }

    template<typename ... Ts>
    bool hasComponents(EntityID entityID)
    {
        EntityData& entityData = m_entityDatas[entityID];
        if (entityData.archetype == nullptr)
            return false;
        return archetypeHasComponents<Ts...>(*entityData.archetype);
    }

    ~ECSWorld() = default;

private:
    template<typename T> inline ComponentID componentID() const { static ComponentID id = s_nextComponentID++; return id; }

    Archetype* archetypeEdgeAdd(Archetype*, ComponentID, utils::uint32 size, const utils::Func<void(void*)>& destructor);
    Archetype* archetypeEdgeRemove(Archetype*, ComponentID);

    utils::uint32 nextAvailableIdx(Archetype*);
    void moveComponents(Archetype* src, utils::uint32 srcIdx, Archetype* dst, utils::uint32 dstIdx);

    template<typename T>
    bool archetypeHasComponents(const Archetype& archetype) const { return archetype.id.contain(componentID<T>()); }

    template<typename T, typename Y, typename ... Ts>
    bool archetypeHasComponents(const Archetype& archetype) const { return archetypeHasComponents<T>() && archetypeHasComponents<Y, Ts...>(); }

    inline static ComponentID s_nextComponentID = 0;

    utils::Array<EntityData> m_entityDatas;
    utils::Set<EntityID> m_availableEntityIDs;
    
    utils::Dictionary<ArchetypeID, utils::UniquePtr<Archetype>> m_archetypes;
    
public:
    ECSWorld& operator = (const ECSWorld&) = delete;
    ECSWorld& operator = (ECSWorld&&)      = delete;
};

}

#endif // ECSWORLD_HPP