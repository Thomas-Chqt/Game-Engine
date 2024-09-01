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

Input::Input(utils::String name) : name(name)
{
}

Input::~Input()
{
}

// ActionInput

void ActionInput::dispatch()
{
    callback();
    triggered = false;
}

ActionInput::ActionInput(utils::String name) : Input(name)
{
}

// StateInput

void StateInput::dispatch()
{
    callback();
}

StateInput::StateInput(utils::String name) : Input(name)
{
}

// RangeInput

void RangeInput::dispatch()
{
    callback(value);
}

RangeInput::RangeInput(utils::String name) : Input(name)
{
}

// Range2DInput

void Range2DInput::dispatch()
{
    callback(value);
}

Range2DInput::Range2DInput(utils::String name) : Input(name)
{
}


}