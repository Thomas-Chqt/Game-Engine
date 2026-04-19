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
#include <tuple>
#include <type_traits>

namespace GE
{

template<ECSWorldLike ECSWorldT, Component... Cs>
struct ecs_view_item
{
    template<typename C>
    using ComponentReference = std::conditional_t<std::is_const_v<ECSWorldT>, const C&, C&>;

    ECSWorldT* world = nullptr;
    typename ECSWorldT::EntityID entityId = INVALID_ENTITY_ID;
    std::tuple<ComponentReference<Cs>...> components;

    ecs_view_item(ECSWorldT* w, typename ECSWorldT::EntityID id, ComponentReference<Cs>... cs)
        : world(w)
        , entityId(id)
        , components(cs...)
    {
    }

    inline operator typename ECSWorldT::EntityID() const { return entityId; }

};

template<std::size_t I, ECSWorldLike ECSWorldT, Component... Cs>
inline auto& get(const ecs_view_item<ECSWorldT, Cs...>& item)
{
    return std::get<I>(item.components);
}

template<ECSWorldLike ECSWorldT, Component... Cs> requires(sizeof...(Cs) > 0)
class basic_ecsView : public std::ranges::view_interface<basic_ecsView<ECSWorldT, Cs...>>
{
private:
    using ArchetypeIterator = std::conditional_t<
        std::is_const_v<ECSWorldT>,
        typename decltype(std::declval<const ECSWorld&>().m_archetypes)::const_iterator,
        typename decltype(std::declval<ECSWorld&>().m_archetypes)::iterator>;

public:
    using value_type = ecs_view_item<ECSWorldT, Cs...>;
    class iterator;
    class reverse_iterator;
    using const_iterator = iterator;
    using const_reverse_iterator = reverse_iterator;

    class iterator
    {
    public:
        using iterator_concept = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = ecs_view_item<ECSWorldT, Cs...>;

        iterator() = default;

        iterator(ECSWorldT* world,
            const std::set<typename ECSWorldT::ComponentID>* predicate,
            ArchetypeIterator arch,
            ArchetypeIterator archEnd)
            : m_world(world)
            , m_predicate(predicate)
            , m_archetypeIt(arch)
            , m_archetypeEnd(archEnd)
        {
            advanceUntilMatch();
        }

        inline value_type operator*() const
        {
            auto& archetype = m_archetypeIt->second;
            return value_type(
                m_world,
                *m_componentIt,
                *archetype.template getComponentPointer<Cs>(m_componentIt.index())...
            );
        }

        inline iterator& operator++()
        {
            ++m_componentIt;
            advanceUntilMatch();
            return *this;
        }

        inline void operator++(int) { ++(*this); }

        inline bool operator==(std::default_sentinel_t) const { return m_archetypeIt == m_archetypeEnd; }

    private:
        ECSWorldT* m_world = nullptr;
        const std::set<typename ECSWorldT::ComponentID>* m_predicate = nullptr;
        ArchetypeIterator m_archetypeIt;
        ArchetypeIterator m_archetypeEnd;
        using ArchetypeType = std::remove_const_t<typename ArchetypeIterator::value_type::second_type>;
        using ComponentIterator = std::conditional_t<std::is_const_v<ECSWorldT>, typename ArchetypeType::const_iterator, typename ArchetypeType::iterator>;
        ComponentIterator m_componentIt;
        ComponentIterator m_componentEnd;

        inline void advanceUntilMatch()
        {
            while (m_archetypeIt != m_archetypeEnd)
            {
                if (!std::ranges::includes(m_archetypeIt->first, *m_predicate))
                {
                    ++m_archetypeIt;
                    m_componentIt = {};
                    m_componentEnd = {};
                    continue;
                }

                if (m_componentIt == ComponentIterator())
                {
                    if constexpr (std::is_const_v<ECSWorldT>)
                    {
                        m_componentIt = m_archetypeIt->second.cbegin();
                        m_componentEnd = m_archetypeIt->second.cend();
                    }
                    else
                    {
                        m_componentIt = m_archetypeIt->second.begin();
                        m_componentEnd = m_archetypeIt->second.end();
                    }
                }

                if (m_componentIt != m_componentEnd)
                    break;

                ++m_archetypeIt;
                m_componentIt = {};
                m_componentEnd = {};
            }
        }
    };

    class reverse_iterator
    {
    public:
        using iterator_concept = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = ecs_view_item<ECSWorldT, Cs...>;

        reverse_iterator() = default;

        reverse_iterator(ECSWorldT* world,
            const std::set<typename ECSWorldT::ComponentID>* predicate,
            ArchetypeIterator archBegin,
            ArchetypeIterator archEnd)
            : m_world(world)
            , m_predicate(predicate)
            , m_archetypeBegin(archBegin)
            , m_archetypeIt(archEnd)
        {
            if (m_archetypeBegin == archEnd)
                return;

            --m_archetypeIt;
            m_componentIdx = m_archetypeIt->second.size() > 0 ? m_archetypeIt->second.size() - 1 : 0;
            retreatUntilMatch();
        }

        inline value_type operator*() const
        {
            auto& archetype = m_archetypeIt->second;
            return value_type(
                m_world,
                archetype.getEntityID(m_componentIdx),
                *archetype.template getComponentPointer<Cs>(m_componentIdx)...
            );
        }

        inline reverse_iterator& operator++()
        {
            if (m_isEnd)
                return *this;

            if (m_componentIdx > 0)
            {
                --m_componentIdx;
                return *this;
            }

            if (m_archetypeIt == m_archetypeBegin)
            {
                m_isEnd = true;
                return *this;
            }

            --m_archetypeIt;
            m_componentIdx = m_archetypeIt->second.size() > 0 ? m_archetypeIt->second.size() - 1 : 0;
            retreatUntilMatch();
            return *this;
        }

        inline void operator++(int) { ++(*this); }

        inline bool operator==(std::default_sentinel_t) const { return m_isEnd; }

    private:
        ECSWorldT* m_world = nullptr;
        const std::set<typename ECSWorldT::ComponentID>* m_predicate = nullptr;
        ArchetypeIterator m_archetypeBegin;
        ArchetypeIterator m_archetypeIt;
        uint64_t m_componentIdx = 0;
        bool m_isEnd = true;

        inline void retreatUntilMatch()
        {
            while (true)
            {
                if (std::ranges::includes(m_archetypeIt->first, *m_predicate))
                {
                    if (m_archetypeIt->second.size() > 0 && m_componentIdx < m_archetypeIt->second.size())
                    {
                        m_isEnd = false;
                        return;
                    }
                }

                if (m_archetypeIt == m_archetypeBegin)
                {
                    m_isEnd = true;
                    return;
                }

                --m_archetypeIt;
                m_componentIdx = m_archetypeIt->second.size() > 0 ? m_archetypeIt->second.size() - 1 : 0;
            }
        }
    };

    basic_ecsView()
        : m_predicate(makePredicate<Cs...>())
    {
    }

    explicit basic_ecsView(ECSWorldT* world)
        : m_world(world)
        , m_predicate(makePredicate<Cs...>())
    {
    }

    basic_ecsView(ECSWorld* world) requires std::is_const_v<ECSWorldT>
        : m_world(world)
        , m_predicate(makePredicate<Cs...>())
    {
    }

    basic_ecsView(const basic_ecsView&) = default;
    basic_ecsView(basic_ecsView&&) = default;

    inline iterator begin() const
    {
        if (m_world == nullptr)
            return iterator();
        if constexpr (std::is_const_v<ECSWorldT>)
            return iterator(m_world, &m_predicate, m_world->m_archetypes.cbegin(), m_world->m_archetypes.cend());
        else
            return iterator(m_world, &m_predicate, m_world->m_archetypes.begin(), m_world->m_archetypes.end());
    }

    inline std::default_sentinel_t end() const { return std::default_sentinel; }
    inline const_iterator cbegin() const { return begin(); }
    inline std::default_sentinel_t cend() const { return std::default_sentinel; }

    inline reverse_iterator rbegin() const
    {
        if (m_world == nullptr)
            return reverse_iterator();
        if constexpr (std::is_const_v<ECSWorldT>)
            return reverse_iterator(m_world, &m_predicate, m_world->m_archetypes.cbegin(), m_world->m_archetypes.cend());
        else
            return reverse_iterator(m_world, &m_predicate, m_world->m_archetypes.begin(), m_world->m_archetypes.end());
    }

    inline std::default_sentinel_t rend() const { return std::default_sentinel; }
    inline reverse_iterator crbegin() const { return rbegin(); }
    inline std::default_sentinel_t crend() const { return std::default_sentinel; }

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
    static std::set<typename ECSWorldT::ComponentID> makePredicate()
    {
        return std::set<typename ECSWorldT::ComponentID>{ ECSWorld::componentID<T>() };
    }

    template<typename T, typename Y, typename... Ys>
    static std::set<typename ECSWorldT::ComponentID> makePredicate()
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
};

template<Component... Cs>
inline basic_ecsView<ECSWorld, Cs...> operator|(ECSWorld& world, basic_ecsView<ECSWorld, Cs...> view)
{
    view.setWorld(&world);
    return view;
}

template<Component... Cs>
inline basic_ecsView<const ECSWorld, Cs...> operator|(const ECSWorld& world, basic_ecsView<const ECSWorld, Cs...> view)
{
    view.setWorld(&world);
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
struct tuple_size<GE::ecs_view_item<ECSWorldT, Cs...>> : std::integral_constant<std::size_t, sizeof...(Cs)> {};

template<std::size_t I, GE::ECSWorldLike ECSWorldT, GE::Component... Cs>
struct tuple_element<I, GE::ecs_view_item<ECSWorldT, Cs...>>
{
    using type = std::conditional_t<std::is_const_v<ECSWorldT>, const std::tuple_element_t<I, std::tuple<Cs...>>, std::tuple_element_t<I, std::tuple<Cs...>>>;
};

} // namespace std

#endif // ECSVIEW_HPP
