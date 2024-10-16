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
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/Types.hpp"
#include <climits>

#define INVALID_ENTITY_ID ULONG_MAX

namespace GE
{

class ECSWorld
{
public:
    using EntityID = utils::uint64;

private:
    template<typename ... Ts> friend class ECSView;

    using ComponentID = utils::uint32;
    using ArchetypeID = utils::Set<ComponentID>;

    using CopyConstructor = utils::Func<void(void* src, void* dst)>;
    using MoveConstructor = utils::Func<void(void* src, void* dst)>;
    using Destructor      = utils::Func<void(void* ptr)>;

public:
    ECSWorld();
    ECSWorld(const ECSWorld&);
    ECSWorld(ECSWorld&&);

    EntityID newEntityID();

    void deleteEntityID(EntityID);
    inline bool isValidEntityID(EntityID id) const { return m_entityDatas.length() > id && m_availableEntityIDs.contain(id) == false; }

    template<typename T, typename ... Args>
    T& emplace(EntityID entityId, Args&& ... args);

    template<typename T>
    void remove(EntityID);

    template<typename T>
    bool has(EntityID);

    template<typename T>
    T& get(EntityID);

    inline utils::uint32 entityCount() { return m_entityDatas.length() - m_availableEntityIDs.size(); }
    inline utils::uint32 archetypeCount() { return m_archetypes.size(); }
    utils::uint32 componentCount();

    ~ECSWorld() = default;

private:
    #include "ECS/Archetype.hpp"

    struct EntityData
    {
        ArchetypeID archetypeId;
        utils::uint64 idx = 0;
    };

    static ComponentID nextComponentID();
    template<typename T> static ComponentID componentID();
    template<typename T> static utils::uint64 componentSize();
    template<typename T> static const CopyConstructor& componentCopyConstructor();
    template<typename T> static const MoveConstructor& componentMoveConstructor();
    template<typename T> static const Destructor& componentDestructor();

    utils::Array<EntityData> m_entityDatas;
    utils::Set<EntityID> m_availableEntityIDs;
    
    utils::Dictionary<ArchetypeID, Archetype> m_archetypes;
    
public:
    ECSWorld& operator = (const ECSWorld&);
    ECSWorld& operator = (ECSWorld&&);
};

template<typename T>
ECSWorld::ComponentID ECSWorld::componentID()
{
    static ComponentID id = nextComponentID();
    return id;
}

template<typename T>
utils::uint64 ECSWorld::componentSize()
{
    static utils::uint64 size = sizeof(T); return size;
}

template<typename T>
const ECSWorld::CopyConstructor& ECSWorld::componentCopyConstructor()
{
    static auto fn = [](void* src, void* dst) { new (dst) T(*(T*)src); };
    return fn;
}

template<typename T>
const ECSWorld::MoveConstructor& ECSWorld::componentMoveConstructor()
{
    static auto fn = [](void* src, void* dst) { new (dst) T(std::move(*(T*)src)); };
    return fn;
}

template<typename T>
const ECSWorld::Destructor& ECSWorld::componentDestructor()
{
    static auto fn = [](void* ptr) { ((T*)ptr)->~T(); };
    return fn;
}

}

#endif // ECSWORLD_HPP