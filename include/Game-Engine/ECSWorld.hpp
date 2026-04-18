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

#define INVALID_ENTITY_ID ULONG_MAX

namespace GE
{

class ECSWorld;

template<typename T>
concept ECSWorldLike = std::is_same_v<std::remove_const_t<T>, GE::ECSWorld>;

template<typename T>
concept Component = std::is_copy_constructible_v<T> && std::is_move_constructible_v<T> && std::is_destructible_v<T>;

class ECSWorld
{
public:
    using EntityID = uint64_t;

    class Iterator;
    class ConstIterator;
    using iterator = Iterator;
    using const_iterator = ConstIterator;
    using reverse_iterator = std::reverse_iterator<Iterator>;
    using const_reverse_iterator = std::reverse_iterator<ConstIterator>;

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

    template<Component T, typename... Args>
    T& emplace(EntityID entityId, Args&&... args);

    template<Component T>
    void remove(EntityID);

    template<Component T>
    bool has(EntityID) const;

    template<Component T>
    T& get(EntityID);

    template<Component T>
    const T& get(EntityID) const;

    inline uint32_t entityCount() { return m_entityDatas.size() - m_availableEntityIDs.size(); }
    inline uint32_t archetypeCount() { return m_archetypes.size(); }
    uint32_t componentCount();

    Iterator begin();
    Iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
    reverse_iterator rbegin();
    reverse_iterator rend();
    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;
    const_reverse_iterator crbegin() const;
    const_reverse_iterator crend() const;

    ~ECSWorld() = default;

private:
    #include "Game-Engine/detail/ECSWorldArchetype.inl"

    struct EntityData
    {
        ArchetypeID archetypeId;
        uint64_t idx = 0;
    };

    static ComponentID nextComponentID();
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

    // friend void to_json(nlohmann::json&, const ECSWorld&);
    // friend void from_json(const nlohmann::json&, ECSWorld&);

public:
    #include "Game-Engine/detail/ECSWorldIterator.inl"
};

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

    entityArch.destructCollum(entityIdx);
    Archetype::moveComponents(entityArch, entityArch.size() - 1, entityArch, entityIdx);
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
T& ECSWorld::get(EntityID entityId)
{
    assert(isValidEntityID(entityId));
    assert(has<T>(entityId));

    Archetype& entityArch = m_archetypes[m_entityDatas[entityId].archetypeId];
    uint64_t& entityIdx = m_entityDatas[entityId].idx;

    return *entityArch.getComponentPointer<T>(entityIdx);
}

template<Component T>
const T& ECSWorld::get(EntityID entityId) const
{
    assert(isValidEntityID(entityId));
    assert(has<T>(entityId));

    const Archetype& entityArch = m_archetypes.at(m_entityDatas[entityId].archetypeId);
    const uint64_t& entityIdx = m_entityDatas[entityId].idx;

    return *entityArch.getComponentPointer<T>(entityIdx);
}

template<Component T>
ECSWorld::ComponentID ECSWorld::componentID()
{
    static ComponentID id = nextComponentID();
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

} // namespace GE

#endif // ECSWORLD_HPP
