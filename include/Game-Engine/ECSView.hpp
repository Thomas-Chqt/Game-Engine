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
#include "Game-Engine/TypeList.hpp"

#include <ranges>
#include <utility>

namespace GE
{

template<Component... Cs>
struct ExcludeComponents {};

template<typename Included, typename Excluded, typename... Terms>
struct ECSViewQuery;

template<Component... Included, Component... Excluded>
struct ECSViewQuery<TypeList<Included...>, TypeList<Excluded...>>
{
    static_assert(sizeof...(Included) > 0, "An ECS view requires at least one included component");

    template<ECSRegistryLike RegistryT>
    using iterator = ECSViewIterator<RegistryT, Included...>;

    static const auto& predicates()
    {
        static const auto predicates = std::pair{archetypeId<Included...>(), archetypeId<Excluded...>()};
        return predicates;
    }
};

template<Component... Included, Component... Excluded, Component C, typename... Rest>
struct ECSViewQuery<TypeList<Included...>, TypeList<Excluded...>, C, Rest...>
    : ECSViewQuery<TypeList<Included..., C>, TypeList<Excluded...>, Rest...>
{};

template<Component... Included, Component... Excluded, Component... Cs, typename... Rest>
struct ECSViewQuery<TypeList<Included...>, TypeList<Excluded...>, ExcludeComponents<Cs...>, Rest...>
    : ECSViewQuery<TypeList<Included...>, TypeList<Excluded..., Cs...>, Rest...>
{};

template<ECSRegistryLike ECSRegistryT, typename... Terms> requires(sizeof...(Terms) > 0)
class basic_ecsView : public std::ranges::view_interface<basic_ecsView<ECSRegistryT, Terms...>>
{
private:
    using Query = ECSViewQuery<TypeList<>, TypeList<>, Terms...>;

public:
    using iterator = typename Query::template iterator<ECSRegistryT>;
    using Iterator = iterator;
    using Predicate = ArchetypeID;

    basic_ecsView() = default;
    basic_ecsView(const basic_ecsView&) = default;
    basic_ecsView(basic_ecsView&&) = default;

    inline ECSRegistryT* registry() const { return m_registry; }

    Iterator begin() const
    {
        if (m_registry == nullptr)
            return {};
        const auto& [required, excluded] = Query::predicates();
        return {m_registry->m_archetypes.begin(), m_registry->m_archetypes.end(), required, excluded};
    }

    Iterator end() const
    {
        if (m_registry == nullptr)
            return {};
        const auto& [required, excluded] = Query::predicates();
        return {m_registry->m_archetypes.end(), m_registry->m_archetypes.end(), required, excluded};
    }

    size_t count() const
    {
        if (m_registry == nullptr)
            return 0;
        const auto& [required, excluded] = Query::predicates();
        size_t output = 0;
        for (const auto& [id, archetype] : m_registry->m_archetypes) {
            if ((id & required) == required && (id & excluded).none())
                output += archetype.size();
        }
        return output;
    }

    ~basic_ecsView() = default;

private:
    ECSRegistryT* m_registry = nullptr;

public:
    basic_ecsView& operator=(const basic_ecsView&) = default;
    basic_ecsView& operator=(basic_ecsView&&) = default;

    template<typename... Ts>
    friend basic_ecsView<ECSRegistry, Ts...> operator|(ECSRegistry& registry, basic_ecsView<ECSRegistry, Ts...> view);

    template<typename... Ts>
    friend basic_ecsView<const ECSRegistry, Ts...> operator|(const ECSRegistry& registry, basic_ecsView<const ECSRegistry, Ts...> view);
};

template<typename... Terms>
basic_ecsView<ECSRegistry, Terms...> operator|(ECSRegistry& registry, basic_ecsView<ECSRegistry, Terms...> view)
{
    view.m_registry = &registry;
    return view;
}

template<typename... Terms>
basic_ecsView<const ECSRegistry, Terms...> operator|(const ECSRegistry& registry, basic_ecsView<const ECSRegistry, Terms...> view)
{
    view.m_registry = &registry;
    return view;
}

template<typename... Terms>
using ECSView = basic_ecsView<ECSRegistry, Terms...>;

template<typename... Terms>
using const_ECSView = basic_ecsView<const ECSRegistry, Terms...>;

} // namespace GE

#endif // ECSVIEW_HPP
