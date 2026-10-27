#pragma once

#include "Game-Engine/Components.hpp"
#include "Game-Engine/ComponentVector.hpp"
#include "Game-Engine/ECSIterator.hpp"
#include "Game-Engine/EntityID.hpp"

#include <algorithm>
#include <cassert>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <ranges>
#include <span>
#include <unordered_map>
#include <utility>

namespace GE
{

using ArchetypeID = std::bitset<maxComponentTypes>;

template<typename ... Ts>
ArchetypeID archetypeId()
{
    ArchetypeID newArchetypeId{};
    (newArchetypeId.set(componentId<Ts>()), ...);
    return newArchetypeId;
}

class GE_API Archetype
{
public:
    using iterator = ECSIterator<uint64_t>;

    Archetype() = default;
    // copy constructor make no sense inside the same ECSRegistry.
    // it is ment to be used only when copying the ECSRegistry
    Archetype(const Archetype&);
    Archetype(Archetype&&) = default;

    template<Component... Ts>
    static Archetype create();

    size_t size() const;

    // return new empty archetypes with the same set components, +/- T
    template<Component T> Archetype addComponentBuffer() const;
    template<Component T> Archetype rmvComponentBuffer() const;

    inline iterator begin() const { return {m_entityIndices.begin()}; }
    inline iterator end() const { return {m_entityIndices.end()}; }

    template<Component T> auto getComponents(this auto&& self);
    template<Component T> auto& getEntityComponent(this auto&& self, EntityID);

    // only work for single component Archetypes
    template<Component T, typename ...Ts>
    void emplaceBack(EntityID, Ts&&... args);

    void erase(EntityID);

    // move 1 entity to the end of an other archetype and emplace the missing component
    // only work when the destination archetype has one more component that the source
    template<Component T, typename ...Ts>
    void moveEntity(EntityID, Archetype& dst, Ts&&... args);

    // move 1 entity to the end of an other archetype
    // only work when the destination archetype has less or the same set of component
    void moveEntity(EntityID, Archetype& dst);

    ~Archetype() = default;

private:
    Archetype(std::unordered_map<ComponentID, std::unique_ptr<AComponentVector>>);

    ArchetypeID m_archetypeId;
    std::unordered_map<ComponentID, std::unique_ptr<AComponentVector>> m_componentBuffers;
    std::unordered_map<EntityID, uint64_t> m_entityIndices;

public:
    Archetype& operator=(const Archetype&);
    Archetype& operator=(Archetype&&) = default;
};

template<Component... Ts>
Archetype Archetype::create()
{
    std::unordered_map<ComponentID, std::unique_ptr<AComponentVector>> componentBuffers;
    (componentBuffers.emplace(componentId<Ts>(), std::make_unique<ComponentVector<Ts>>()), ...);
    return {std::move(componentBuffers)};
}

template<Component T>
Archetype Archetype::addComponentBuffer() const
{
    assert(m_componentBuffers.contains(componentId<T>()) == false);
    auto newComponentBuffers = m_componentBuffers
        | std::views::transform([](const auto& pair){ return std::make_pair(pair.first, pair.second->create()); })
        | std::ranges::to<std::unordered_map>();
    newComponentBuffers.emplace(componentId<T>(), std::make_unique<ComponentVector<T>>());
    return {std::move(newComponentBuffers)};
}

template<Component T>
Archetype Archetype::rmvComponentBuffer() const
{
    assert(m_componentBuffers.contains(componentId<T>()));
    auto newComponentBuffers = m_componentBuffers
        | std::views::transform([](const auto& pair){ return std::make_pair(pair.first, pair.second->create()); })
        | std::ranges::to<std::unordered_map>();
    newComponentBuffers.erase(newComponentBuffers.find(componentId<T>()));
    return {std::move(newComponentBuffers)};
}

template<Component T, typename ...Ts>
void Archetype::emplaceBack(EntityID id, Ts&&... args)
{
    assert(archetypeId<T>() == m_archetypeId);
    [[maybe_unused]] auto [it, inserted] = m_entityIndices.emplace(id, size());
    assert(inserted);
    auto& aCompVec = m_componentBuffers.at(componentId<T>());
    auto* compVec = static_cast<ComponentVector<T>*>(aCompVec.get());
    compVec->vector().emplace_back(std::forward<Ts>(args)...);
}

template<Component T>
auto Archetype::getComponents(this auto&& self)
{
    assert(self.m_componentBuffers.contains(componentId<T>()));
    using Self = std::remove_reference_t<decltype(self)>;
    using Vector = std::conditional_t<std::is_const_v<Self> || std::is_same_v<T, EntityID>, const ComponentVector<T>, ComponentVector<T>>;
    auto& buffer = static_cast<Vector&>(*self.m_componentBuffers.at(componentId<T>()));
    return std::span(buffer.vector());
}

template<Component T>
auto& Archetype::getEntityComponent(this auto&& self, EntityID id)
{
    assert(self.m_entityIndices.contains(id));
    return self.template getComponents<T>()[self.m_entityIndices.at(id)];
}

template<Component T, typename ...Ts>
void Archetype::moveEntity(EntityID id, Archetype& dst, Ts&&... args)
{
    assert(&dst != this);
    assert(m_entityIndices.contains(id));
    assert(!dst.m_entityIndices.contains(id));
    assert(!m_archetypeId.test(componentId<T>()));
    assert((m_archetypeId | archetypeId<T>()) == dst.m_archetypeId);
    auto& newComponents = static_cast<ComponentVector<T>&>(*dst.m_componentBuffers.at(componentId<T>()));
    newComponents.vector().emplace_back(std::forward<Ts>(args)...);
    const auto idx = m_entityIndices.at(id);
    const auto last = size() - 1;
    // The hash-map iteration order is unrelated to the dense component rows.
    const auto& ids = static_cast<const ComponentVector<EntityID>&>(*m_componentBuffers.at(componentId<EntityID>())).vector();
    const auto lastId = ids.back();
    dst.m_entityIndices.emplace(id, dst.m_entityIndices.size());
    for (auto& [compId, buffer] : m_componentBuffers)
        buffer->moveComponent(idx, *dst.m_componentBuffers.at(compId));
    if (idx != last)
        m_entityIndices.at(lastId) = idx;
    m_entityIndices.erase(id);
}

}
