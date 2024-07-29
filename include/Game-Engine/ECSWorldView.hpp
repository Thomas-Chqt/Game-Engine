/*
 * ---------------------------------------------------
 * ECSWorldView.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/28 18:24:19
 * ---------------------------------------------------
 */

#ifndef ECSWORLDVIEW_HPP
#define ECSWORLDVIEW_HPP

#include "ECSWorld.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/Types.hpp"

namespace GE
{

template<typename ... Ts>
class ECSWorldView
{
public:
    ECSWorldView()                    = delete;
    ECSWorldView(const ECSWorldView&) = delete;
    ECSWorldView(ECSWorldView&&)      = delete;
    
    ECSWorldView(const ECSWorld& world) : m_world(world)
    {
    }

    utils::uint32 count() const
    {
        utils::uint32 output = 0;
        for (auto& [_, archetype] : m_world.m_archetypes)
        {
            if (m_world.archetypeHasComponents<Ts...>(*archetype))
                output += archetype->entryCount - archetype->availableIndices.size();
        }
        return output;
    }

    void foreach(const utils::Func<void(Ts&...)>& func) const
    {
        for (auto& [_, archetype] : m_world.m_archetypes)
        {
            if (m_world.archetypeHasComponents<Ts...>(*archetype))
            {
                for (utils::uint32 i = 0; i < archetype->entryCount; i++)
                {
                    if (archetype->availableIndices.contain(i))
                        continue;
                    func(*(Ts*)(archetype->rows[m_world.componentID<Ts>()].buffer) ...);
                }
            }
        }
    }

    ~ECSWorldView() = default;

private:
    const ECSWorld& m_world;

public:
    ECSWorldView& operator = (const ECSWorldView&) = delete;
    ECSWorldView& operator = (ECSWorldView&&)      = delete;
};

}

#endif // ECSWORLDVIEW_HPP