/*
 * ---------------------------------------------------
 * Input.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/24 17:29:30
 * ---------------------------------------------------
 */

#include "InputManager/Input.hpp"
#include "InputManager/Mapper.hpp" // IWYU pragma: keep

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

void ActionInput::reset()
{
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

void StateInput::reset()
{
    triggered = false;
}

StateInput::StateInput(utils::String name) : Input(name)
{
}

// RangeInput

void RangeInput::dispatch()
{
    callback(value);
}

void RangeInput::reset()
{
    value = 0.0F;
    triggered = false;
}

RangeInput::RangeInput(utils::String name) : Input(name)
{
}

// Range2DInput

void Range2DInput::dispatch()
{
    callback(value);
}

void Range2DInput::reset()
{
    value = { 0.0F, 0.0F };
    triggered = false;
}

Range2DInput::Range2DInput(utils::String name) : Input(name)
{
}


}