/*
 * ---------------------------------------------------
 * Input.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/24 17:29:30
 * ---------------------------------------------------
 */

#include "Game-Engine/Input.hpp"
#include "Game-Engine/Mapper.hpp" // IWYU pragma: keep

namespace GE
{

Input::Input(utils::String name) : m_name(name)
{
}

void Input::setMapper0(utils::UniquePtr<IMapper>&& map)
{
    m_mappers[0] = std::move(map);
}

void Input::setMapper1(utils::UniquePtr<IMapper>&& map)
{
    m_mappers[1] = std::move(map);
}


void Input::onInputEvent(gfx::InputEvent& event)
{
    if (hasCallback() == false)
        return;
    for (utils::uint8 i = 0; i < 2; i++)
    {
        if (m_mappers[i] == false)
            continue;

        m_mappers[i]->onInputEvent(event);

        if (event.processed())
            return;
    }
}

Input::~Input()
{
}

// ActionInput

ActionInput::ActionInput(utils::String name) : Input(name)
{
}

// StateInput

StateInput::StateInput(utils::String name) : Input(name)
{
}

// RangeInput

RangeInput::RangeInput(utils::String name) : Input(name)
{
}

// Range2DInput

Range2DInput::Range2DInput(utils::String name) : Input(name)
{
}


}