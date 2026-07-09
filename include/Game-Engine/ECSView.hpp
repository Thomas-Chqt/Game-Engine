/*
 * ---------------------------------------------------
 * ECSView.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/28 18:24:19
 * ---------------------------------------------------
 */

#ifndef ECSVIEW_HPP
#define ECSVIEW_HPP

#include "Game-Engine/ECSWorld.hpp"

#include <bitset>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <tuple> // IWYU pragma: keep
#include <type_traits> // IWYU pragma: keep
#include <utility> // IWYU pragma: keep

namespace GE
{

template<ECSWorldLike ECSWorldT, Component... Cs>
struct ecs_view_item
{
    template<typename C>
    using ComponentReference = std::conditional_t<std::is_const_v<ECSWorldT>, const C&, C&>;

    EntityID entityId = INVALID_ENTITY_ID;
    std::tuple<ComponentReference<Cs>...> components;

    template<std::size_t I>
    inline decltype(auto) get() const { return std::get<I>(components); }

    inline operator EntityID() const { return entityId; }
};

template<ECSWorldLike ECSWorldT, Component... Cs> requires(sizeof...(Cs) > 0)
class basic_ecsView : public std::ranges::view_interface<basic_ecsView<ECSWorldT, Cs...>>
{
public:
    class Iterator;
    using iterator = Iterator;
    using Predicate = std::bitset<std::remove_const_t<ECSWorldT>::maxComponentTypes>;

public:
    basic_ecsView() : m_predicate(makePredicate<Cs...>()) {}
    basic_ecsView(const basic_ecsView&) = default;
    basic_ecsView(basic_ecsView&&) = default;

    inline ECSWorldT* world() const { return m_world; }

    inline Iterator begin() const;
    inline std::default_sentinel_t end() const { return std::default_sentinel; }

    template<Component... Ts> requires(sizeof...(Ts) > 0)
    basic_ecsView without() const
    {
        basic_ecsView view(*this);
        view.m_excludedPredicate |= makePredicate<Ts...>();
        return view;
    }

    uint32_t count() const
    {
        uint32_t output = 0;
        for (const auto& [archetypeId, archetype] : m_world->m_archetypes)
        {
            if (matches(archetypeId))
                output += archetype.size();
        }
        return output;
    }

    ~basic_ecsView() = default;

private:
    ECSWorldT* m_world = nullptr;
    Predicate m_predicate;
    Predicate m_excludedPredicate;

    inline void setWorld(ECSWorldT* world) { m_world = world; }

    inline bool matches(const Predicate& archetypeId) const
    {
        return (archetypeId & m_predicate) == m_predicate
            && (archetypeId & m_excludedPredicate).none();
    }

    template<typename T>
    static Predicate makePredicate()
    {
        Predicate predicate;
        predicate.set(ECSWorld::componentID<T>());
        return predicate;
    }

    template<typename T, typename Y, typename... Ys>
    static Predicate makePredicate()
    {
        auto predicate = makePredicate<T>();
        auto tail = makePredicate<Y, Ys...>();
        predicate |= tail;
        return predicate;
    }

public:
    basic_ecsView& operator=(const basic_ecsView&) = default;
    basic_ecsView& operator=(basic_ecsView&&) = default;

    template<Component... Ts>
    friend basic_ecsView<ECSWorld, Ts...> operator|(ECSWorld& world, basic_ecsView<ECSWorld, Ts...> view);

    template<Component... Ts>
    friend basic_ecsView<const ECSWorld, Ts...> operator|(const ECSWorld& world, basic_ecsView<const ECSWorld, Ts...> view);

public:
    #include "Game-Engine/ECSViewIterator.inl"
};

template<ECSWorldLike ECSWorldT, Component... Cs> requires(sizeof...(Cs) > 0)
inline typename basic_ecsView<ECSWorldT, Cs...>::Iterator basic_ecsView<ECSWorldT, Cs...>::begin() const
{
    if (m_world == nullptr)
        return Iterator();

    auto archetypeIt = m_world->m_archetypes.begin();
    while (archetypeIt != m_world->m_archetypes.end())
    {
        if (matches(archetypeIt->first))
        {
            auto entityIt = archetypeIt->second.begin();
            if (entityIt != archetypeIt->second.end())
                return Iterator(m_world, m_predicate, m_excludedPredicate, archetypeIt, entityIt);
        }
        ++archetypeIt;
    }

    return Iterator(m_world, m_predicate, m_excludedPredicate, m_world->m_archetypes.end(), {});
}

template<Component... Cs>
basic_ecsView<ECSWorld, Cs...> operator|(ECSWorld& world, basic_ecsView<ECSWorld, Cs...> view)
{
    view.m_world = &world;
    return view;
}

template<Component... Cs>
basic_ecsView<const ECSWorld, Cs...> operator|(const ECSWorld& world, basic_ecsView<const ECSWorld, Cs...> view)
{
    view.m_world = &world;
    return view;
}

template<Component... Cs>
using ECSView = basic_ecsView<ECSWorld, Cs...>;

template<Component... Cs>
using const_ECSView = basic_ecsView<const ECSWorld, Cs...>;

} // namespace GE

namespace std
{

template<GE::ECSWorldLike ECSWorldT, GE::Component... Cs>
struct tuple_size<GE::ecs_view_item<ECSWorldT, Cs...>> : std::tuple_size<decltype(std::declval<GE::ecs_view_item<ECSWorldT, Cs...>>().components)> {};

template<std::size_t I, GE::ECSWorldLike ECSWorldT, GE::Component... Cs>
struct tuple_element<I, GE::ecs_view_item<ECSWorldT, Cs...>>
{
    using type = std::tuple_element_t<I, decltype(std::declval<GE::ecs_view_item<ECSWorldT, Cs...>>().components)>;
};

} // namespace std

#endif // ECSVIEW_HPP
