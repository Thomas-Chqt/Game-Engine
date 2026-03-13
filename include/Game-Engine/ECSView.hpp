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
#include "Game-Engine/Entity.hpp"

#include <functional>
#include <set>
#include <cstdint>
#include <algorithm>
#include <type_traits>

namespace GE
{

template<ECSWorldLike ECSWorldT, Component... Cs> requires(sizeof...(Cs) > 0)
class basic_ecsView
{
public:
    basic_ecsView(ECSWorldT* world)
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

    uint32_t count() const
    {
        uint32_t output = 0;
        for (auto& [archetypeId, archetype] : m_world->m_archetypes) {
            if (std::ranges::includes(archetypeId, m_predicate))
                output += archetype.size();
        }
        return output;
    }

    void onFirst(const std::function<void(ECSWorld::EntityID, std::conditional_t<std::is_const_v<ECSWorldT>, const Cs&, Cs&> ...)>& f) const
    {
        for (auto& [archetypeId, archetype] : m_world->m_archetypes)
        {
            if (std::ranges::includes(archetypeId, m_predicate))
            {
                for (uint32_t idx = 0; idx < archetype.size(); idx++)
                {
                    f(archetype.getEntityID(idx), *archetype.template getComponentPointer<Cs>(idx) ...);
                    return;
                }
            }
        }
    }

    void onFirst(const std::function<void(basic_entity<ECSWorldT>, std::conditional_t<std::is_const_v<ECSWorldT>, const Cs&, Cs&> ...)>& f) const
    {
        onFirst([&](typename ECSWorldT::EntityId entityId, Cs& ... components){
            basic_entity<ECSWorldT> entity = {
                .world = m_world,
                .entityId = entityId
            };
            f(entity, components...);
        });
    }

    void onEach(const std::function<void(ECSWorld::EntityID, std::conditional_t<std::is_const_v<ECSWorldT>, const Cs&, Cs&> ...)>& f) const
    {
        for (auto& [archetypeId, archetype] : m_world->m_archetypes)
        {
            if (std::ranges::includes(archetypeId, m_predicate))
            {
                for (uint32_t idx = 0; idx < archetype.size(); idx++)
                    f(archetype.getEntityID(idx), *archetype.template getComponentPointer<Cs>(idx) ...);
            }
        }

    }

    void onEach(const std::function<void(basic_entity<ECSWorldT>, std::conditional_t<std::is_const_v<ECSWorldT>, const Cs&, Cs&> ...)>& f) const
    {
        onEach([&](typename ECSWorldT::EntityId entityId, Cs& ... components){
            basic_entity<ECSWorldT> entity = {
                .world = m_world,
                .entityId = entityId
            };
            f(entity, components...);
        });
    }

    ~basic_ecsView() = default;

private:
    ECSWorldT* m_world = nullptr;
    std::set<typename ECSWorldT::ComponentID> m_predicate;

    template<typename T>
    static std::set<typename ECSWorldT::ComponentID> makePredicate()
    {
        return std::set<typename ECSWorldT::ComponentID>{ ECSWorld::componentID<T>() };
    }

    template<typename T, typename Y, typename... Ys>
    static std::set<typename ECSWorldT::ComponentID> makePredicate()
    {
        auto predicate = makePredicate<T>();
        predicate.insert_range(makePredicate<Y, Ys...>());
        return predicate;
    }

public:
    basic_ecsView& operator = (const basic_ecsView&) = default;
    basic_ecsView& operator = (basic_ecsView&&) = default;
};

template<Component... Cs>
using ECSView = basic_ecsView<ECSWorld, Cs...>;

template<Component... Cs>
using const_ECSView = basic_ecsView<const ECSWorld, Cs...>;

} // namespace GE

#endif // ECSVIEW_HPP
