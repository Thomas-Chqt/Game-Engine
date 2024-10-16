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

#include "ECS/ECSWorld.hpp"
#include "ECS/Entity.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/Types.hpp"

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

    utils::uint32 count();

    void onFirst(const utils::Func<void(Entity, Ts& ...)>&);
    void onEach(const utils::Func<void(Entity, Ts& ...)>&);

    ~ECSView() = default;

private:
    ECSWorld& m_world;
    const utils::Set<ECSWorld::ComponentID> m_predicate;

    template<typename T>
    static utils::Set<ECSWorld::ComponentID> makePredicate()
    {
        return utils::Set<ECSWorld::ComponentID>{ ECSWorld::componentID<T>() };
    }

    template<typename T, typename Y, typename ... Ys>
    static utils::Set<ECSWorld::ComponentID> makePredicate()
    {
        return makePredicate<T>() + makePredicate<Y, Ys...>(); 
    }

public:
    ECSView& operator = (const ECSView&) = delete;
    ECSView& operator = (ECSView&&)      = delete;
};

}

#endif // ECSVIEW_HPP