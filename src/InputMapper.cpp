/*
 * ---------------------------------------------------
 * InputMapper.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/InputMapper.hpp"
#include "Game-Engine/Input.hpp" // IWYU pragma: keep

#include <cmath>
#include <glm/geometric.hpp>

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
        if (event.processed())
            return;
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
        if (event.processed())
            return;
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
        if (event.processed())
            return;
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
    , triggerValue(desc.triggerValue)
{
}

void InputMapper<KeyboardButton, Range2DInput>::onInputEvent(InputEvent& event)
{
    if (input == nullptr)
        return;

    event.dispatch<KeyDownEvent>([&](const KeyDownEvent& keyDownEvent)
    {
        if (event.processed() || keyDownEvent.isRepeat())
            return;

        if (keyDownEvent.keyCode() == static_cast<int>(xPos))
            rawValue.x += xScale;
        else if (keyDownEvent.keyCode() == static_cast<int>(xNeg))
            rawValue.x -= xScale;
        else if (keyDownEvent.keyCode() == static_cast<int>(yPos))
            rawValue.y += yScale;
        else if (keyDownEvent.keyCode() == static_cast<int>(yNeg))
            rawValue.y -= yScale;
        else
            return;

        event.markAsProcessed();
    });

    event.dispatch<KeyUpEvent>([&](const KeyUpEvent& keyUpEvent)
    {
        if (keyUpEvent.keyCode() == static_cast<int>(xPos))
            rawValue.x -= xScale;
        else if (keyUpEvent.keyCode() == static_cast<int>(xNeg))
            rawValue.x += xScale;
        else if (keyUpEvent.keyCode() == static_cast<int>(yPos))
            rawValue.y -= yScale;
        else if (keyUpEvent.keyCode() == static_cast<int>(yNeg))
            rawValue.y += yScale;
        else
            return;

        event.markAsProcessed();
    });

    if (event.processed())
    {
        if (std::abs(rawValue.x) > 0.0f || std::abs(rawValue.y) > 0.0f)
            input->value = glm::normalize(rawValue);
        else
            input->value = {0.0f, 0.0f};
        input->triggered = std::abs(input->value.x) >= triggerValue || std::abs(input->value.y) >= triggerValue;
    }
}

}
