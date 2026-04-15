/*
 * ---------------------------------------------------
 * InputMapper.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/InputMapper.hpp"
#include "Game-Engine/Input.hpp" // IWYU pragma: keep

namespace GE
{

InputMapper<KeyboardButton, ActionInput>::InputMapper(KeyboardButton button)
    : button(button)
{
}

void InputMapper<KeyboardButton, ActionInput>::onInputEvent(InputEvent& event)
{
    if (input == nullptr)
        return;
    event.dispatch<KeyDownEvent>([this, &event](const KeyDownEvent& keyDownEvent)
    {
        if (keyDownEvent.keyCode() == static_cast<int>(button) && keyDownEvent.isRepeat() == false)
        {
            input->triggered = true;
            event.markAsProcessed();
        }
    });
}

InputMapper<KeyboardButton, StateInput>::InputMapper(KeyboardButton button)
    : button(button)
{
}

void InputMapper<KeyboardButton, StateInput>::onInputEvent(InputEvent& event)
{
    if (input == nullptr)
        return;
    event.dispatch<KeyDownEvent>([this, &event](const KeyDownEvent& keyDownEvent)
    {
        if (keyDownEvent.keyCode() == static_cast<int>(button))
        {
            input->triggered = true;
            event.markAsProcessed();
        }
    });
    event.dispatch<KeyUpEvent>([this, &event](const KeyUpEvent& keyUpEvent)
    {
        if (keyUpEvent.keyCode() == static_cast<int>(button))
        {
            input->triggered = false;
            event.markAsProcessed();
        }
    });
}

InputMapper<KeyboardButton, RangeInput>::InputMapper(KeyboardButton button, float scale)
    : button(button)
    , scale(scale)
{
}

void InputMapper<KeyboardButton, RangeInput>::onInputEvent(InputEvent& event)
{
    if (input == nullptr)
        return;
    event.dispatch<KeyDownEvent>([this, &event](const KeyDownEvent& keyDownEvent)
    {
        if (keyDownEvent.keyCode() == static_cast<int>(button))
        {
            input->value = scale;
            input->triggered = true;
            event.markAsProcessed();
        }
    });
    event.dispatch<KeyUpEvent>([this, &event](const KeyUpEvent& keyUpEvent)
    {
        if (keyUpEvent.keyCode() == static_cast<int>(button))
        {
            input->value = 0.0f;
            input->triggered = false;
            event.markAsProcessed();
        }
    });
}

InputMapper<KeyboardButton, Range2DInput>::InputMapper(const Descriptor& desc)
    : xPos(desc.xPos) , xNeg(desc.xNeg) , xScale(desc.xScale)
    , yPos(desc.yPos) , yNeg(desc.yNeg) , yScale(desc.yScale)
{
}

void InputMapper<KeyboardButton, Range2DInput>::onInputEvent(InputEvent& event)
{
    if (input == nullptr)
        return;
    event.dispatch<KeyDownEvent>([this, &event](const KeyDownEvent& keyDownEvent)
    {
        if (keyDownEvent.keyCode() == static_cast<int>(xPos))
        {
            input->value.first = xScale;
            input->triggered = true;
            event.markAsProcessed();
        }
        else if (keyDownEvent.keyCode() == static_cast<int>(xNeg))
        {
            input->value.first = -xScale;
            input->triggered = true;
            event.markAsProcessed();
        }
        else if (keyDownEvent.keyCode() == static_cast<int>(yPos))
        {
            input->value.second = yScale;
            input->triggered = true;
            event.markAsProcessed();
        }
        else if (keyDownEvent.keyCode() == static_cast<int>(yNeg))
        {
            input->value.second = -yScale;
            input->triggered = true;
            event.markAsProcessed();
        }
    });

    event.dispatch<KeyUpEvent>([this, &event](const KeyUpEvent& keyUpEvent)
    {
        if (keyUpEvent.keyCode() == static_cast<int>(xPos) || keyUpEvent.keyCode() == static_cast<int>(xNeg))
            input->value.first = 0.0f;
        if (keyUpEvent.keyCode() == static_cast<int>(yPos) || keyUpEvent.keyCode() == static_cast<int>(yNeg))
            input->value.second = 0.0f;

        if (input->value.first == 0.0f && input->value.second == 0.0f)
            input->triggered = false;

        if (keyUpEvent.keyCode() == static_cast<int>(xPos)
            || keyUpEvent.keyCode() == static_cast<int>(xNeg)
            || keyUpEvent.keyCode() == static_cast<int>(yPos)
            || keyUpEvent.keyCode() == static_cast<int>(yNeg)) event.markAsProcessed();
    });
}

}
