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
#include <utility>
#include <climits>

#define INVALID_ENTITY_ID ULONG_MAX

namespace GE
{

class Archetype;

class ECSWorld
{
public:
    using EntityID = utils::uint64;

private:
    using ComponentID = utils::uint32;
    using ArchetypeID = utils::Set<ComponentID>;

private:
    template<typename ... Ts> friend class ECSView;
    friend class Archetype;

public:
    ECSWorld();
    ECSWorld(const ECSWorld&) = delete;
    ECSWorld(ECSWorld&&)      = delete;

    EntityID newEntity();
    void deleteEntity(EntityID);
    bool isValid(EntityID id) { return m_entityDatas.length() > id && m_availableEntityIDs.contain(id) == false; }

    inline utils::uint32 entityCount() { return m_entityDatas.length() - m_availableEntityIDs.size(); }
    inline utils::uint32 archetypeCount() { return m_archetypes.size(); }
    utils::uint32 componentCount();

    template<typename T, typename ... Args>
    T& emplace(EntityID, Args&& ... args);

    template<typename T>
    inline void remove(EntityID entityId) { remove(entityId, componentID<T>()); }

    template<typename T>
    inline bool has(EntityID entityId) { return has(entityId, componentID<T>()); }

    template<typename T>
    inline T& get(EntityID entityId) { return *(T*)get(entityId, componentID<T>()); }

    ~ECSWorld();

private:
    struct EntityData
    {
        Archetype* archetype = nullptr;
        utils::uint32 idx = 0;
    };

    inline static ComponentID nextComponentID() { static ComponentID id = 0; return id++; };
    template<typename T> inline static ComponentID componentID() { static ComponentID id = nextComponentID(); return id; }

    void* emplace(EntityID, ComponentID, utils::uint32 size, const utils::Func<void(void*)>& constructor, const utils::Func<void(void*)>& destructor);
    void remove(EntityID, ComponentID);

    bool has(EntityID, ComponentID);
    void* get(EntityID, ComponentID);

    utils::Array<EntityData> m_entityDatas;
    utils::Set<EntityID> m_availableEntityIDs;
    
    utils::UniquePtr<Archetype> m_emptyArchetype;
    utils::Dictionary<ArchetypeID, utils::UniquePtr<Archetype>> m_archetypes;
    
public:
    ECSWorld& operator = (const ECSWorld&) = delete;
    ECSWorld& operator = (ECSWorld&&)      = delete;
};

template<typename T, typename ... Args>
T& ECSWorld::emplace(EntityID entityId, Args&& ... args)
{
    utils::uint32 size = sizeof(T);
    auto constructor = [&](void* ptr){ new (ptr) T{std::forward<Args>(args)...}; };
    auto destructor = [](void* ptr){ ((T*)ptr)->~T(); };
    return *(T*)emplace(entityId, componentID<T>(), size, constructor, destructor);
}

}

#endif // ECSWORLD_HPP