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

#include <functional>
#include <set>
#include <cstdint>
#include <algorithm>

namespace GE
{

template<typename ... Ts>
class ECSView
{
public:
    ECSView()               = delete;
    ECSView(const ECSView&) = delete;
    ECSView(ECSView&&)      = delete;

    ECSView(ECSWorld& world) : m_world(world), m_predicate(makePredicate<Ts...>()) {}

    uint32_t count() const;

    void onFirst(const std::function<void(ECSWorld::EntityID, Ts& ...)>&);
    // void onFirst(const std::function<void(Entity, Ts& ...)>&);

    void onEach(const std::function<void(ECSWorld::EntityID, Ts& ...)>&);
    // void onEach(const std::function<void(Entity, Ts& ...)>&);

    ~ECSView() = default;

private:
    ECSWorld& m_world;
    const std::set<ECSWorld::ComponentID> m_predicate;

    template<typename T>
    static std::set<ECSWorld::ComponentID> makePredicate()
    {
        return std::set<ECSWorld::ComponentID>{ ECSWorld::componentID<T>() };
    }

    template<typename T, typename Y, typename ... Ys>
    static std::set<ECSWorld::ComponentID> makePredicate()
    {
        auto predicate = makePredicate<T>();
        predicate.insert_range(makePredicate<Y, Ys...>());
        return predicate;
    }

public:
    ECSView& operator = (const ECSView&) = delete;
    ECSView& operator = (ECSView&&)      = delete;
};

template<typename ... Ts>
uint32_t ECSView<Ts...>::count() const
{
    uint32_t output = 0;
    for (auto& [archetypeId, archetype] : m_world.m_archetypes) {
        if (std::ranges::includes(archetypeId, m_predicate))
            output += archetype.size();
    }
    return output;
}

template<typename ... Ts>
void ECSView<Ts...>::onFirst(const std::function<void(ECSWorld::EntityID, Ts& ...)>& func)
{
    for (auto& [archetypeId, archetype] : m_world.m_archetypes)
    {
        if (std::ranges::includes(archetypeId, m_predicate))
        {
            for (uint32_t idx = 0; idx < archetype.size(); idx++)
            {
                func(archetype.getEntityID(idx), *archetype.template getComponentPointer<Ts>(idx) ...);
                return;
            }
        }
    }
}

// template<typename ... Ts>
// void ECSView<Ts...>::onFirst(const std::function<void(Entity, Ts& ...)>& func)
// {
//     for (auto& [archetypeId, archetype] : m_world.m_archetypes)
//     {
//         if (archetypeId.contain(m_predicate))
//         {
//             for (uint32_t idx = 0; idx < archetype.size(); idx++)
//             {
//                 func(Entity(m_world, archetype.getEntityID(idx)), *(Ts*)archetype.template getComponentPointer<Ts>(idx) ...);
//                 return;
//             }
//         }
//     }
// }

template<typename ... Ts>
void ECSView<Ts...>::onEach(const std::function<void(ECSWorld::EntityID, Ts& ...)>& func)
{
    for (auto& [archetypeId, archetype] : m_world.m_archetypes)
    {
        if (std::ranges::includes(archetypeId, m_predicate))
        {
            for (uint32_t idx = 0; idx < archetype.size(); idx++)
                func(archetype.getEntityID(idx), *archetype.template getComponentPointer<Ts>(idx) ...);
        }
    }
}

// template<typename ... Ts>
// void ECSView<Ts...>::onEach(const std::function<void(Entity, Ts& ...)>& func)
// {
//     for (auto& [archetypeId, archetype] : m_world.m_archetypes)
//     {
//         if (archetypeId.contain(m_predicate))
//         {
//             for (uint32_t idx = 0; idx < archetype.size(); idx++)
//                 func(Entity(m_world, archetype.getEntityID(idx)), *(Ts*)archetype.template getComponentPointer<Ts>(idx) ...);
//         }
//     }
// }

template<typename ... Ts>
class const_ECSView
{
public:
    const_ECSView()                     = delete;
    const_ECSView(const const_ECSView&) = delete;
    const_ECSView(const_ECSView&&)      = delete;

    const_ECSView(const ECSWorld& world) : m_world(world), m_predicate(makePredicate<Ts...>()) {}

    uint32_t count() const;

    void onFirst(const std::function<void(ECSWorld::EntityID, const Ts& ...)>&);
    // void onFirst(const std::function<void(const Entity, const Ts& ...)>&);
    void onEach(const std::function<void(ECSWorld::EntityID, const Ts& ...)>&);
    // void onEach(const std::function<void(const Entity, const Ts& ...)>&);

    ~const_ECSView() = default;

private:
    const ECSWorld& m_world;
    const std::set<ECSWorld::ComponentID> m_predicate;

    template<typename T>
    static std::set<ECSWorld::ComponentID> makePredicate()
    {
        return std::set<ECSWorld::ComponentID>{ ECSWorld::componentID<T>() };
    }

    template<typename T, typename Y, typename ... Ys>
    static std::set<ECSWorld::ComponentID> makePredicate()
    {
        auto predicate = makePredicate<T>();
        predicate.insert_range(makePredicate<Y, Ys...>());
        return predicate;
    }

public:
    const_ECSView& operator = (const const_ECSView&) = delete;
    const_ECSView& operator = (const_ECSView&&)      = delete;
};

template<typename ... Ts>
uint32_t const_ECSView<Ts...>::count() const
{
    uint32_t output = 0;
    for (auto& [archetypeId, archetype] : m_world.m_archetypes) {
        if (std::ranges::includes(archetypeId, m_predicate))
            output += archetype.size();
    }
    return output;
}

template<typename ... Ts>
void const_ECSView<Ts...>::onFirst(const std::function<void(ECSWorld::EntityID, const Ts& ...)>& func)
{
    for (auto& [archetypeId, archetype] : m_world.m_archetypes)
    {
        if (std::ranges::includes(archetypeId, m_predicate))
        {
            for (uint32_t idx = 0; idx < archetype.size(); idx++)
            {
                func(archetype.getEntityID(idx), *archetype.template getComponentPointer<Ts>(idx) ...);
                return;
            }
        }
    }
}

// template<typename ... Ts>
// void const_ECSView<Ts...>::onFirst(const std::function<void(const Entity, const Ts& ...)>& func)
// {
//     for (auto& [archetypeId, archetype] : m_world.m_archetypes)
//     {
//         if (archetypeId.contain(m_predicate))
//         {
//             for (uint32_t idx = 0; idx < archetype.size(); idx++)
//             {
//                 func(Entity(const_cast<ECSWorld&>(m_world), archetype.getEntityID(idx)), *(Ts*)archetype.template getComponentPointer<Ts>(idx) ...);
//                 return;
//             }
//         }
//     }
// }

template<typename ... Ts>
void const_ECSView<Ts...>::onEach(const std::function<void(ECSWorld::EntityID, const Ts& ...)>& func)
{
    for (auto& [archetypeId, archetype] : m_world.m_archetypes)
    {
        if (std::ranges::includes(archetypeId, m_predicate))
        {
            for (uint32_t idx = 0; idx < archetype.size(); idx++)
                func(archetype.getEntityID(idx), *archetype.template getComponentPointer<Ts>(idx) ...);
        }
    }
}

// template<typename ... Ts>
// void const_ECSView<Ts...>::onEach(const std::function<void(const Entity, const Ts& ...)>& func)
// {
//     for (auto& [archetypeId, archetype] : m_world.m_archetypes)
//     {
//         if (archetypeId.contain(m_predicate))
//         {
//             for (uint32_t idx = 0; idx < archetype.size(); idx++)
//                 func(Entity(const_cast<ECSWorld&>(m_world), archetype.getEntityID(idx)), *(Ts*)archetype.template getComponentPointer<Ts>(idx) ...);
//         }
//     }
// }

}

#endif // ECSVIEW_HPP
