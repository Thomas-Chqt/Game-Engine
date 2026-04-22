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

#include "Game-Engine/Export.hpp"

#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <set>
#include <functional>
#include <map>
#include <iterator>
#include <type_traits>
#include <typeinfo>

#define INVALID_ENTITY_ID ULONG_MAX

namespace GE
{

class ECSWorld;

template<typename T>
concept ECSWorldLike = std::is_same_v<std::remove_const_t<T>, GE::ECSWorld>;

template<typename T>
concept Component = std::is_copy_constructible_v<T> && std::is_move_constructible_v<T> && std::is_destructible_v<T>;

class GE_API ECSWorld
{
public:
    using EntityID = uint64_t;

    struct Iterator;
    using iterator = Iterator;

private:
    template<ECSWorldLike ECSWorldT, Component  ... Cs> requires(sizeof...(Cs) > 0) friend class basic_ecsView;

    using ComponentID = uint32_t;
    using ArchetypeID = std::set<ComponentID>;

    using CopyConstructor = std::function<void(void* src, void* dst)>;
    using MoveConstructor = std::function<void(void* src, void* dst)>;
    using Destructor = std::function<void(void* ptr)>;

public:
    ECSWorld();
    ECSWorld(const ECSWorld&) = default;
    ECSWorld(ECSWorld&&) = default;

    EntityID newEntityID();
    void registerEntityID(ECSWorld::EntityID);
    void deleteEntityID(EntityID);
    inline bool isValidEntityID(EntityID id) const { return id < m_entityDatas.size() && m_availableEntityIDs.contains(id) == false; }

    template<Component T, typename... Args> T& emplace(EntityID entityId, Args&&... args);
    template<Component T> void remove(EntityID);
    template<Component T> bool has(EntityID) const;
    template<Component T> auto& get(this auto&& self, EntityID);

    inline uint32_t entityCount() { return m_entityDatas.size() - m_availableEntityIDs.size(); }
    inline uint32_t archetypeCount() { return m_archetypes.size(); }
    uint32_t componentCount();

    inline Iterator begin() const;
    inline std::default_sentinel_t end() const { return std::default_sentinel; }

    ~ECSWorld() = default;

private:
    #include "Game-Engine/Archetype.inl"

    struct EntityData
    {
        ArchetypeID archetypeId;
        uint64_t idx = 0;
    };

    static ComponentID nextComponentID();
    static ComponentID componentID(const std::type_info&);
    template<Component T> static ComponentID componentID();

    template<Component T> static uint64_t componentSize();
    template<Component T> static CopyConstructor componentCopyConstructor();
    template<Component T> static MoveConstructor componentMoveConstructor();
    template<Component T> static Destructor componentDestructor();

    std::vector<EntityData> m_entityDatas;
    std::set<EntityID> m_availableEntityIDs;

    std::map<ArchetypeID, Archetype> m_archetypes;

public:
    ECSWorld& operator=(const ECSWorld&) = default;
    ECSWorld& operator=(ECSWorld&&) = default;

public:
    #include "Game-Engine/ECSWorldIterator.inl"
};

inline ECSWorld::Iterator ECSWorld::begin() const
{
    auto id = 0;
    while (m_availableEntityIDs.contains(id))
        id++;
    return Iterator(this, id);
}

template<Component T, typename... Args>
T& ECSWorld::emplace(EntityID entityId, Args&&... args)
{
    assert(isValidEntityID(entityId));
    assert(has<T>(entityId) == false);

    ArchetypeID& entityArchID = m_entityDatas[entityId].archetypeId;
    Archetype& entityArch = m_archetypes[entityArchID];
    uint64_t& entityIdx = m_entityDatas[entityId].idx;

    ArchetypeID dstArchID = entityArchID;
    dstArchID.insert(componentID<T>());
    auto it = m_archetypes.find(dstArchID);
    if (it == m_archetypes.end())
    {
        Archetype newArchetype = entityArch.duplicateRowTypes();
        newArchetype.addRowType<T>();
        auto [newIt, err] = m_archetypes.insert(std::make_pair(dstArchID, std::move(newArchetype)));
        assert(err);
        it = newIt;
    }
    Archetype& dstArchetype = it->second;
    uint64_t dstIdx = dstArchetype.allocateCollum();

    Archetype::moveComponents(entityArch, entityIdx, dstArchetype, dstIdx);

    m_entityDatas[entityArch.getEntityID(entityArch.size() - 1)].idx = entityIdx;

    if (entityIdx != entityArch.size() - 1)
    {
        entityArch.destructCollum(entityIdx);
        Archetype::moveComponents(entityArch, entityArch.size() - 1, entityArch, entityIdx);
    }
    entityArch.destructCollum(entityArch.size() - 1);
    entityArch.freeLastCollum();

    entityArchID = dstArchID;
    entityIdx = dstIdx;

    T* componentPtr = m_archetypes[entityArchID].getComponentPointer<T>(entityIdx);
    new (componentPtr) T(std::forward<Args>(args)...);
    return *componentPtr;
}

template<Component T>
void ECSWorld::remove(EntityID entityId)
{
    assert(isValidEntityID(entityId));
    assert(has<T>(entityId));

    ArchetypeID& entityArchID = m_entityDatas[entityId].archetypeId;
    Archetype& entityArch = m_archetypes[entityArchID];
    uint64_t& entityIdx = m_entityDatas[entityId].idx;

    ArchetypeID dstArchID = entityArchID;
    dstArchID.erase(componentID<T>());
    auto it = m_archetypes.find(dstArchID);
    if (it == m_archetypes.end())
    {
        Archetype newArchetype = entityArch.duplicateRowTypes();
        newArchetype.rmvRowType<T>();
        auto [newIt, err] = m_archetypes.insert(std::make_pair(dstArchID, std::move(newArchetype)));
        assert(err);
        it = newIt;
    }
    Archetype& dstArchetype = it->second;
    uint64_t dstIdx = dstArchetype.allocateCollum();

    Archetype::moveComponents(entityArch, entityIdx, dstArchetype, dstIdx);

    m_entityDatas[entityArch.getEntityID(entityArch.size() - 1)].idx = entityIdx;

    if (entityIdx != entityArch.size() - 1)
    {
        entityArch.destructCollum(entityIdx);
        Archetype::moveComponents(entityArch, entityArch.size() - 1, entityArch, entityIdx);
    }
    entityArch.destructCollum(entityArch.size() - 1);
    entityArch.freeLastCollum();

    entityArchID = dstArchID;
    entityIdx = dstIdx;
}

template<Component T>
bool ECSWorld::has(EntityID entityId) const
{
    assert(isValidEntityID(entityId));
    return m_entityDatas[entityId].archetypeId.contains(componentID<T>());
}

template<Component T>
auto& ECSWorld::get(this auto&& self, EntityID entityId)
{
    assert(self.isValidEntityID(entityId));
    assert(self.template has<T>(entityId));

    auto& entityArch = self.m_archetypes.at(self.m_entityDatas[entityId].archetypeId);
    uint64_t entityIdx = self.m_entityDatas.at(entityId).idx;

    return *entityArch.template getComponentPointer<T>(entityIdx);
}

template<Component T>
ECSWorld::ComponentID ECSWorld::componentID()
{
    static const ComponentID id = componentID(typeid(T));
    return id;
}

template<Component T>
uint64_t ECSWorld::componentSize()
{
    uint64_t size = sizeof(T);
    return size;
}

template<Component T>
ECSWorld::CopyConstructor ECSWorld::componentCopyConstructor()
{
    auto fn = [](void* src, void* dst) { new (dst) T(*(T*)src); };
    return fn;
}

template<Component T>
ECSWorld::MoveConstructor ECSWorld::componentMoveConstructor()
{
    auto fn = [](void* src, void* dst) { new (dst) T(std::move(*(T*)src)); };
    return fn;
}

template<Component T>
ECSWorld::Destructor ECSWorld::componentDestructor()
{
    auto fn = [](void* ptr) { ((T*)ptr)->~T(); };
    return fn;
}

template<typename T>
void ECSWorld::Archetype::addRowType()
{
    m_rows.insert(std::make_pair(componentID<T>(), Row{
        operator new (componentSize<T>() * m_capacity),
        componentSize<T>(),
        componentCopyConstructor<T>(),
        componentMoveConstructor<T>(),
        componentDestructor<T>(),
    }));
}

template<typename T>
void ECSWorld::Archetype::rmvRowType()
{
    Row& row = m_rows.at(componentID<T>());
    if (row.buffer != nullptr)
    {
        for (uint64_t i = 0; i < m_size; i++)
            row.destructor(static_cast<std::byte*>(row.buffer) + (row.componentSize * i));
        operator delete(row.buffer);
    }
    m_rows.erase(componentID<T>());
}

template<Component T>
auto* ECSWorld::Archetype::getComponentPointer(this auto&& self, uint64_t idx)
{
    using Self = std::remove_reference_t<decltype(self)>;
    using ComponentPtr = std::conditional_t<std::is_const_v<Self>, const T*, T*>;
    auto& row = self.m_rows.at(componentID<T>());
    return static_cast<ComponentPtr>(row.buffer) + idx;
}

} // namespace GE

#endif // ECSWORLD_HPP
