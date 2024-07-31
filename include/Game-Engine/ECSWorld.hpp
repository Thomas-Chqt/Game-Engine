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
    using ComponentID = utils::uint32;
    using ArchetypeID = utils::Set<ComponentID>;

private:
    inline static ComponentID nextComponentID() { static ComponentID id = 0; return id++; };
    template<typename T> inline static ComponentID componentID() { static ComponentID id = nextComponentID(); return id; }

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
        utils::Array<utils::uint64> entityIds;
        utils::Dictionary<ComponentID, Row> rows;
        
        utils::Dictionary<ComponentID, Archetype*> edgeAdd;
        utils::Dictionary<ComponentID, Archetype*> edgeRemove;

        inline Archetype(ArchetypeID id) : id(std::move(id)) {}

        template<typename T> inline bool hasComponents() const { return id.contain(componentID<T>()); }
        template<typename T, typename Y, typename ... Ts> inline bool hasComponents() const { return hasComponents<T>() && hasComponents<Y, Ts...>(); }
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

    inline utils::uint64 entityCount() const { return m_entityDatas.length() - m_availableEntityIDs.size(); }
    inline utils::uint64 archetypeCount() const { return m_archetypes.size(); }
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

        newArchetype->entityIds[newArchetypeIdx] = entityID;
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

    template<typename T> inline const T& getComponent(EntityID entityID) const { return (const T&)const_cast<ECSWorld*>(this)->getComponent<T>(entityID); }

    template<typename ... Ts>
    bool hasComponents(EntityID entityID) const
    {
        const EntityData& entityData = m_entityDatas[entityID];
        if (entityData.archetype == nullptr)
            return false;
        return entityData.archetype->hasComponents<Ts...>();
    }

    ~ECSWorld() = default;

private:

    Archetype* archetypeEdgeAdd(Archetype*, ComponentID, utils::uint32 size, const utils::Func<void(void*)>& destructor);
    Archetype* archetypeEdgeRemove(Archetype*, ComponentID);

    utils::uint32 nextAvailableIdx(Archetype*);
    void moveComponents(Archetype* src, utils::uint32 srcIdx, Archetype* dst, utils::uint32 dstIdx);

    utils::Array<EntityData> m_entityDatas;
    utils::Set<EntityID> m_availableEntityIDs;
    
    utils::Dictionary<ArchetypeID, utils::UniquePtr<Archetype>> m_archetypes;
    
public:
    ECSWorld& operator = (const ECSWorld&) = delete;
    ECSWorld& operator = (ECSWorld&&)      = delete;

public:
    class Entity
    {
    public:
        Entity()              = delete;
        Entity(const Entity&) = delete;
        Entity(Entity&&)      = delete;

        inline Entity(ECSWorld& world) : world(&world), id(world.createEntity()) {}

        template<typename T> inline void add(const T& component) { world->addComponent(id, component); }
        template<typename T> inline void remove() { world->removeComponent<T>(id); }
        template<typename T> inline T& get() { return world->getComponent<T>(id); }
        template<typename ... Ts> inline bool has() { return world->hasComponents<Ts...>(id); }

        inline ~Entity() { world->deleteEntity(id); }

    private:
        ECSWorld* world;
        EntityID id;

    public:
        Entity& operator = (const Entity&) = delete;
        Entity& operator = (Entity&&)      = delete;
    };

    template<typename ... Ts>
    class View
    {
    public:
        View()            = delete;
        View(const View&) = delete;
        View(View&&)      = delete;

        View(ECSWorld& world) : m_world(world)
        {
        }

        utils::uint32 count() const
        {
            utils::uint32 output = 0;
            for (auto& [_, archetype] : m_world.m_archetypes)
            {
                if (archetype->template hasComponents<Ts...>())
                    output += archetype->entryCount - archetype->availableIndices.size();
            }
            return output;
        }

        void foreach(const utils::Func<void(Ts&...)>& func) const
        {
            for (auto& [_, archetype] : m_world.m_archetypes)
            {
                if (archetype->template hasComponents<Ts...>())
                {
                    for (utils::uint32 i = 0; i < archetype->entryCount; i++)
                    {
                        if (archetype->availableIndices.contain(i))
                            continue;
                        func(((Ts*)(archetype->rows[m_world.componentID<Ts>()].buffer))[i] ...);
                    }
                }
            }
        }

        void onFirst(const utils::Func<void(Ts&...)>& func) const
        {
            for (auto& [_, archetype] : m_world.m_archetypes)
            {
                if (archetype->template hasComponents<Ts...>())
                {
                    for (utils::uint32 i = 0; i < archetype->entryCount; i++)
                    {
                        if (archetype->availableIndices.contain(i))
                            continue;
                        func(((Ts*)(archetype->rows[m_world.componentID<Ts>()].buffer))[i] ...);
                        return;
                    }
                }
            }
        }

    private:
        ECSWorld& m_world;

    public:
        View& operator = (const View&) = delete;
        View& operator = (View&&)      = delete;
    };
};

}

#endif // ECSWORLD_HPP
