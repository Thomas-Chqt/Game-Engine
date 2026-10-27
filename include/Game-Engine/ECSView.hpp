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

#include "Game-Engine/ECSRegistry.hpp"
#include "Game-Engine/ECSViewIterator.hpp"

#include <bitset>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <tuple> // IWYU pragma: keep
#include <type_traits> // IWYU pragma: keep
#include <utility> // IWYU pragma: keep

namespace GE
{

template<ECSRegistryLike ECSRegistryT, Component... Cs> requires(sizeof...(Cs) > 0)
class basic_ecsView : public std::ranges::view_interface<basic_ecsView<ECSRegistryT, Cs...>>
{
public:
    using iterator = ECSViewIterator<ECSRegistryT, Cs...>;
    using Iterator = iterator;
    using Predicate = ArchetypeID;

public:
    basic_ecsView() : m_predicate(makePredicate<Cs...>()) {}
    basic_ecsView(const basic_ecsView&) = default;
    basic_ecsView(basic_ecsView&&) = default;

    inline ECSRegistryT* registry() const { return m_registry; }

    inline Iterator begin() const;
    inline Iterator end() const;

    template<Component... Ts> requires(sizeof...(Ts) > 0)
    basic_ecsView without() const
    {
        basic_ecsView view(*this);
        view.m_excludedPredicate |= makePredicate<Ts...>();
        return view;
    }

    size_t count() const
    {
        if (m_registry == nullptr)
            return 0;
        size_t output = 0;
        for (const auto& [archetypeId, archetype] : m_registry->m_archetypes)
        {
            if (matches(archetypeId))
                output += archetype.size();
        }
        return output;
    }

    ~basic_ecsView() = default;

private:
    ECSRegistryT* m_registry = nullptr;
    Predicate m_predicate;
    Predicate m_excludedPredicate;

    inline void setRegistry(ECSRegistryT* registry) { m_registry = registry; }

    inline bool matches(const Predicate& archetypeId) const
    {
        return (archetypeId & m_predicate) == m_predicate
            && (archetypeId & m_excludedPredicate).none();
    }

    template<typename T>
    static Predicate makePredicate()
    {
        Predicate predicate;
        predicate.set(componentId<T>());
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
    friend basic_ecsView<ECSRegistry, Ts...> operator|(ECSRegistry& registry, basic_ecsView<ECSRegistry, Ts...> view);

    template<Component... Ts>
    friend basic_ecsView<const ECSRegistry, Ts...> operator|(const ECSRegistry& registry, basic_ecsView<const ECSRegistry, Ts...> view);

};

template<ECSRegistryLike ECSRegistryT, Component... Cs> requires(sizeof...(Cs) > 0)
inline typename basic_ecsView<ECSRegistryT, Cs...>::Iterator basic_ecsView<ECSRegistryT, Cs...>::begin() const
{
    if (m_registry == nullptr)
        return {};
    return {m_registry->m_archetypes.begin(), m_registry->m_archetypes.end(), m_predicate, m_excludedPredicate};
}

template<ECSRegistryLike ECSRegistryT, Component... Cs> requires(sizeof...(Cs) > 0)
inline typename basic_ecsView<ECSRegistryT, Cs...>::Iterator basic_ecsView<ECSRegistryT, Cs...>::end() const
{
    if (m_registry == nullptr)
        return {};
    return {m_registry->m_archetypes.end(), m_registry->m_archetypes.end(), m_predicate, m_excludedPredicate};
}

template<Component... Cs>
basic_ecsView<ECSRegistry, Cs...> operator|(ECSRegistry& registry, basic_ecsView<ECSRegistry, Cs...> view)
{
    view.m_registry = &registry;
    return view;
}

template<Component... Cs>
basic_ecsView<const ECSRegistry, Cs...> operator|(const ECSRegistry& registry, basic_ecsView<const ECSRegistry, Cs...> view)
{
    view.m_registry = &registry;
    return view;
}

template<Component... Cs>
using ECSView = basic_ecsView<ECSRegistry, Cs...>;

template<Component... Cs>
using const_ECSView = basic_ecsView<const ECSRegistry, Cs...>;

} // namespace GE

#endif // ECSVIEW_HPP
