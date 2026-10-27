#pragma once

#include "Game-Engine/ECSRegistry.hpp"

#include <cassert>
#include <cstddef>
#include <iterator>
#include <span>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace GE
{

template<ECSRegistryLike ECSRegistryT, Component... Cs>
struct ecs_view_item
{
    template<typename C>
    using ComponentReference = std::conditional_t<std::is_const_v<ECSRegistryT> || std::is_same_v<C, EntityID>, const C&, C&>;

    EntityID entityId;
    std::tuple<ComponentReference<Cs>...> components;

    template<std::size_t I>
    inline decltype(auto) get() const { return std::get<I>(components); }

    inline operator EntityID() const { return entityId; }
};

template<ECSRegistryLike ECSRegistryT, Component... Cs>
class ECSViewIterator
{
public:
    using iterator_concept = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = ecs_view_item<ECSRegistryT, Cs...>;
    using ArchetypeMap = std::unordered_map<ArchetypeID, Archetype>;
    using ArchetypeMapIterator = std::conditional_t<std::is_const_v<ECSRegistryT>, ArchetypeMap::const_iterator, ArchetypeMap::iterator>;

private:
    template<Component C>
    using ComponentSpan = std::span<std::conditional_t<std::is_const_v<ECSRegistryT> || std::is_same_v<C, EntityID>, const C, C>>;

    ArchetypeMapIterator m_archetypeIt;
    ArchetypeMapIterator m_end;
    ArchetypeID m_predicate;
    ArchetypeID m_excludedPredicate;
    std::span<const EntityID> m_entityIds;
    std::tuple<ComponentSpan<Cs>...> m_components;
    size_t m_row = 0;

public:
    ECSViewIterator() = default;
    ECSViewIterator(const ECSViewIterator&) = default;
    ECSViewIterator(ECSViewIterator&&) = default;

    ECSViewIterator(ArchetypeMapIterator, ArchetypeMapIterator end, const ArchetypeID& predicate, const ArchetypeID& excludedPredicate);

    ~ECSViewIterator() = default;

private:
    void findArchetype();

public:
    ECSViewIterator& operator=(const ECSViewIterator&) = default;
    ECSViewIterator& operator=(ECSViewIterator&&) = default;

    value_type operator*() const;

    ECSViewIterator& operator++();
    ECSViewIterator operator++(int);

    bool operator==(const ECSViewIterator&) const;
};

template<ECSRegistryLike ECSRegistryT, Component... Cs>
ECSViewIterator<ECSRegistryT, Cs...>::ECSViewIterator(ArchetypeMapIterator archetypeIt, ArchetypeMapIterator end, const ArchetypeID& predicate, const ArchetypeID& excludedPredicate)
    : m_archetypeIt(archetypeIt)
    , m_end(end)
    , m_predicate(predicate)
    , m_excludedPredicate(excludedPredicate)
{
    findArchetype();
}

template<ECSRegistryLike ECSRegistryT, Component... Cs>
void ECSViewIterator<ECSRegistryT, Cs...>::findArchetype()
{
    while (m_archetypeIt != m_end) {
        const auto& id = m_archetypeIt->first;
        auto& archetype = m_archetypeIt->second;
        if ((id & m_predicate) == m_predicate && (id & m_excludedPredicate).none() && archetype.size() != 0) {
            m_entityIds = archetype.template getComponents<EntityID>();
            m_components = {archetype.template getComponents<Cs>()...};
            return;
        }
        ++m_archetypeIt;
    }
}

template<ECSRegistryLike ECSRegistryT, Component... Cs>
auto ECSViewIterator<ECSRegistryT, Cs...>::operator*() const -> value_type
{
    assert(m_archetypeIt != m_end);
    assert(m_row < m_entityIds.size());
    return std::apply([&](const auto&... components) {
        return value_type{
            .entityId = m_entityIds[m_row],
            .components = {components[m_row]...}
        };
    }, m_components);
}

template<ECSRegistryLike ECSRegistryT, Component... Cs>
auto ECSViewIterator<ECSRegistryT, Cs...>::operator++() -> ECSViewIterator&
{
    assert(m_archetypeIt != m_end);
    ++m_row;
    if (m_row == m_entityIds.size()) {
        m_row = 0;
        ++m_archetypeIt;
        findArchetype();
    }
    return *this;
}

template<ECSRegistryLike ECSRegistryT, Component... Cs>
auto ECSViewIterator<ECSRegistryT, Cs...>::operator++(int) -> ECSViewIterator
{
    auto previous = *this;
    ++(*this);
    return previous;
}

template<ECSRegistryLike ECSRegistryT, Component... Cs>
bool ECSViewIterator<ECSRegistryT, Cs...>::operator==(const ECSViewIterator& other) const
{
    return m_archetypeIt == other.m_archetypeIt && m_row == other.m_row;
}

} // namespace GE

namespace std
{

template<GE::ECSRegistryLike ECSRegistryT, GE::Component... Cs>
struct tuple_size<GE::ecs_view_item<ECSRegistryT, Cs...>> : std::tuple_size<decltype(std::declval<GE::ecs_view_item<ECSRegistryT, Cs...>>().components)> {};

template<std::size_t I, GE::ECSRegistryLike ECSRegistryT, GE::Component... Cs>
struct tuple_element<I, GE::ecs_view_item<ECSRegistryT, Cs...>>
{
    using type = std::tuple_element_t<I, decltype(std::declval<GE::ecs_view_item<ECSRegistryT, Cs...>>().components)>;
};

} // namespace std
