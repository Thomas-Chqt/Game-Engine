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

namespace GE
{

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

// template<std::size_t I, ECSWorldLike ECSWorldT, Component... Cs>
// inline auto& get(const ecs_view_item<ECSWorldT, Cs...>& item)
// {
//     return std::get<I>(item.components);
// }

// namespace std
// {

// template<GE::ECSWorldLike ECSWorldT, GE::Component... Cs>
// struct tuple_size<GE::ecs_view_item<ECSWorldT, Cs...>> : std::integral_constant<std::size_t, sizeof...(Cs)> {};

// template<std::size_t I, GE::ECSWorldLike ECSWorldT, GE::Component... Cs>
// struct tuple_element<I, GE::ecs_view_item<ECSWorldT, Cs...>>
// {
//     using type = std::conditional_t<std::is_const_v<ECSWorldT>, const std::tuple_element_t<I, std::tuple<Cs...>>, std::tuple_element_t<I, std::tuple<Cs...>>>;
// };

// } // namespace std

#endif // ECSVIEW_HPP
