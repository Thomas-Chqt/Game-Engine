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

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <set>
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

    typename ECSWorldT::EntityID entityId = INVALID_ENTITY_ID;
    std::tuple<ComponentReference<Cs>...> components;

    template<std::size_t I>
    inline decltype(auto) get() const { return std::get<I>(components); }

    inline operator typename ECSWorldT::EntityID() const { return entityId; }
};

template<ECSWorldLike ECSWorldT, Component... Cs> requires(sizeof...(Cs) > 0)
class basic_ecsView : public std::ranges::view_interface<basic_ecsView<ECSWorldT, Cs...>>
{
public:
    class Iterator;
    using iterator = Iterator;
    using Predicate = std::set<typename ECSWorldT::ComponentID>;

public:
    basic_ecsView() : m_predicate(makePredicate<Cs...>()) {}
    basic_ecsView(const basic_ecsView&) = default;
    basic_ecsView(basic_ecsView&&) = default;

    inline Iterator begin() const;
    inline std::default_sentinel_t end() const { return std::default_sentinel; }

    uint32_t count() const
    {
        uint32_t output = 0;
        for (const auto& [archetypeId, archetype] : m_world->m_archetypes)
        {
            if (std::ranges::includes(archetypeId, m_predicate))
                output += archetype.size();
        }
        return output;
    }

    ~basic_ecsView() = default;

private:
    ECSWorldT* m_world = nullptr;
    std::set<typename ECSWorldT::ComponentID> m_predicate;

    inline void setWorld(ECSWorldT* world) { m_world = world; }

    template<typename T>
    static Predicate makePredicate()
    {
        return std::set<typename ECSWorldT::ComponentID>{ ECSWorld::componentID<T>() };
    }

    template<typename T, typename Y, typename... Ys>
    static Predicate makePredicate()
    {
        auto predicate = makePredicate<T>();
        auto tail = makePredicate<Y, Ys...>();
        predicate.insert(tail.begin(), tail.end());
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
        if (std::ranges::includes(archetypeIt->first, m_predicate))
        {
            auto entityIt = archetypeIt->second.begin();
            if (entityIt != archetypeIt->second.end())
                return Iterator(m_world, m_predicate, archetypeIt, entityIt);
        }
        ++archetypeIt;
    }

    return Iterator(m_world, m_predicate, m_world->m_archetypes.end(), {});
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
