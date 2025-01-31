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

utils::UniquePtr<Input> ActionInput::clone() const
{
    auto ipt = utils::makeUnique<ActionInput>(name);
    ipt->triggered = triggered;
    ipt->mappers[0] = mappers[0]->clone(ipt);
    ipt->mappers[1] = mappers[1]->clone(ipt);
    ipt->callback = callback;
    return ipt.staticCast<Input>();
}

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

utils::UniquePtr<Input> StateInput::clone() const
{
    auto ipt = utils::makeUnique<StateInput>(name);
    ipt->triggered = triggered;
    ipt->mappers[0] = mappers[0]->clone(ipt);
    ipt->mappers[1] = mappers[1]->clone(ipt);
    ipt->callback = callback;
    return ipt.staticCast<Input>();
}

void StateInput::dispatch()
{
    callback();
}

void StateInput::reset()
{
}

StateInput::StateInput(utils::String name) : Input(name)
{
}

// RangeInput

utils::UniquePtr<Input> RangeInput::clone() const
{
    auto ipt = utils::makeUnique<RangeInput>(name);
    ipt->triggered = triggered;
    ipt->mappers[0] = mappers[0]->clone(ipt);
    ipt->mappers[1] = mappers[1]->clone(ipt);
    ipt->callback = callback;
    ipt->value = value;
    return ipt.staticCast<Input>();
}

void RangeInput::dispatch()
{
    callback(value);
}

void RangeInput::reset()
{
}

RangeInput::RangeInput(utils::String name) : Input(name)
{
}

// Range2DInput

utils::UniquePtr<Input> Range2DInput::clone() const
{
    auto ipt = utils::makeUnique<Range2DInput>(name);
    ipt->triggered = triggered;
    ipt->mappers[0] = mappers[0]->clone(ipt);
    ipt->mappers[1] = mappers[1]->clone(ipt);
    ipt->callback = callback;
    ipt->value = value;
    return ipt.staticCast<Input>();
}

void Range2DInput::dispatch()
{
    callback(value);
}

void Range2DInput::reset()
{
}

Range2DInput::Range2DInput(utils::String name) : Input(name)
{
}


}