#pragma once

#include "Game-Engine/Export.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/ECSIterator.hpp"
#include "Game-Engine/Archetype.hpp"
#include "Game-Engine/EntityID.hpp"

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#include <uuid.h>

#include <cassert>
#include <cstddef>
#include <unordered_map>

namespace GE
{

class ECSRegistry;

template<typename T>
concept ECSRegistryLike = std::is_same_v<std::remove_const_t<T>, ECSRegistry>;

template<ECSRegistryLike RegistryT>
struct basic_entity;

using Entity = basic_entity<ECSRegistry>;
using const_Entity = basic_entity<const ECSRegistry>;

class GE_API ECSRegistry
{
public:
    using Iterator = ECSIterator<ArchetypeID>;
    using iterator = Iterator;

private:
    template<ECSRegistryLike RegistryT, typename... Terms> requires(sizeof...(Terms) > 0) friend class basic_ecsView;

public:
    ECSRegistry() = default;
    ECSRegistry(const ECSRegistry&) = default;
    ECSRegistry(ECSRegistry&&) = default;

    EntityID newEntityID();
    void registerEntityID(EntityID);
    void deleteEntityID(EntityID);
    bool isValidEntityID(EntityID) const;

    EntityIDMap insert(const ECSRegistry&);

    template<Component T, typename... Args>
    T& emplace(EntityID entityId, Args&&... args);

    template<Component T>
    void remove(EntityID);

    template<Component T>
    bool has(EntityID) const;

    template<Component T>
    auto& get(this auto&& self, EntityID);

    size_t archetypeCount() const;
    size_t entityCount() const;
    size_t componentCount() const;

    iterator begin() const;
    iterator end() const;

    ~ECSRegistry() = default;

private:
    std::unordered_map<ArchetypeID, Archetype> m_archetypes;
    std::unordered_map<EntityID, ArchetypeID> m_entityArchetypes;

public:
    ECSRegistry& operator=(const ECSRegistry&) = default;
    ECSRegistry& operator=(ECSRegistry&&) = default;
};

template<Component T, typename... Args>
T& ECSRegistry::emplace(EntityID id, Args&&... args)
{
    static_assert(!std::is_same_v<T, EntityID>, "Entity IDs are owned by the registry");
    assert(isValidEntityID(id));
    assert(!has<T>(id));
    auto& srcId = m_entityArchetypes.at(id);
    auto& src = m_archetypes.at(srcId);
    auto dstId = srcId;
    dstId.set(componentId<T>());
    auto it = m_archetypes.find(dstId);
    if (it == m_archetypes.end())
        it = m_archetypes.emplace(dstId, src.template createEmptyWith<T>()).first;
    moveEntity<T>(id, src, it->second, std::forward<Args>(args)...);
    srcId = dstId;
    return it->second.template getEntityComponent<T>(id);
}

template<Component T>
void ECSRegistry::remove(EntityID id)
{
    static_assert(!std::is_same_v<T, EntityID>, "Entity IDs are owned by the registry");
    assert(isValidEntityID(id));
    assert(has<T>(id));
    auto& srcId = m_entityArchetypes.at(id);
    auto& src = m_archetypes.at(srcId);
    auto dstId = srcId;
    dstId.reset(componentId<T>());
    auto it = m_archetypes.find(dstId);
    if (it == m_archetypes.end())
        it = m_archetypes.emplace(dstId, src.template createEmptyWithout<T>()).first;
    moveEntity(id, src, it->second);
    srcId = dstId;
}

template<Component T>
bool ECSRegistry::has(EntityID id) const
{
    assert(isValidEntityID(id));
    return m_entityArchetypes.at(id).test(componentId<T>());
}

template<Component T>
auto& ECSRegistry::get(this auto&& self, EntityID id)
{
    assert(self.isValidEntityID(id));
    assert(self.template has<T>(id));
    return self.m_archetypes.at(self.m_entityArchetypes.at(id)).template getEntityComponent<T>(id);
}

} // namespace GE
