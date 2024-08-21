/*
 * ---------------------------------------------------
 * InputContext.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/19 10:44:40
 * ---------------------------------------------------
 */

#include "Game-Engine/InputContext.hpp"
#include "Game-Engine/RawInputs.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/Types.hpp"
#include "Graphics/KeyCodes.hpp"
#include "UtilsCPP/Macros.hpp"
#include <cassert>

namespace GE
{

RawInput keyCodeToRawInput(int keyCode)
{
    switch (keyCode)
    {
    case ESC_KEY:
        return RawInput::esc_key;
    case ONE_KEY:
        UNREACHABLE
    case TWO_KEY:
        UNREACHABLE
    case W_KEY:
        return RawInput::w_key;
    case A_KEY:
        return RawInput::a_key;
    case S_KEY:
        return RawInput::s_key;
    case D_KEY:
        return RawInput::d_key;
    case SPACE_KEY:
        UNREACHABLE
    case UP_KEY:
        return RawInput::up_key;
    case LEFT_KEY:
        return RawInput::left_key;
    case DOWN_KEY:
        return RawInput::down_key;
    case RIGHT_KEY:
        return RawInput::right_key;
    case LEFT_SHIFT_KEY:
        UNREACHABLE
    default:
        UNREACHABLE
    }
}

RawInput mouseCodeToRawInput(int mouseCode)
{
    switch (mouseCode)
    {
    case MOUSE_L:
        UNREACHABLE
    case MOUSE_R:
        UNREACHABLE
    default:
        UNREACHABLE
    }
}

InputContext::InputContext(const utils::String& name)
    : m_name(name)
{
    for (utils::uint32 i = 0; i < (utils::uint32)RawInput::count; i++)
        new (m_mappers + i) InputMapper();
}

void InputContext::onEvent(gfx::Event& event)
{
    if (event.dispatch<gfx::KeyDownEvent>(utils::Func<void(gfx::KeyDownEvent&)>(*this, &InputContext::onKeyDownEvent)))
        return;
    if (event.dispatch<gfx::KeyUpEvent>(utils::Func<void(gfx::KeyUpEvent&)>(*this, &InputContext::onKeyUpEvent)))
        return;
    if (event.dispatch<gfx::ScrollEvent>(utils::Func<void(gfx::ScrollEvent&)>(*this, &InputContext::onScrollEvent)))
        return;
    if (event.dispatch<gfx::MouseMoveEvent>(utils::Func<void(gfx::MouseMoveEvent&)>(*this, &InputContext::onMouseMoveEvent)))
        return;
    if (event.dispatch<gfx::MouseDownEvent>(utils::Func<void(gfx::MouseDownEvent&)>(*this, &InputContext::onMouseDownEvent)))
        return;
    if (event.dispatch<gfx::MouseUpEvent>(utils::Func<void(gfx::MouseUpEvent&)>(*this, &InputContext::onMouseUpEvent)))
        return;
}

void InputContext::onKeyDownEvent(gfx::KeyDownEvent& event)
{
    InputMapper& mapper = m_mappers[(unsigned long)keyCodeToRawInput(event.keyCode())];
    Input* mappedInput = mapper.mappedInput;
    if (mappedInput == nullptr)
        return;

    if ((mappedInput->type == Input::Type::action && event.isRepeat() == false) ||
        (mappedInput->type == Input::Type::state))
    {
        mappedInput->value = mapper.scale;
        mappedInput->triggered = true;
    }
    event.process();
}

void InputContext::onKeyUpEvent(gfx::KeyUpEvent& event)
{
    InputMapper& mapper = m_mappers[(unsigned long)keyCodeToRawInput(event.keyCode())];
    Input* mappedInput = mapper.mappedInput;
    if (mappedInput == nullptr)
        return;
    
    if (mappedInput->type == Input::Type::state)
        mappedInput->triggered = false;

    event.process();
}

void InputContext::onScrollEvent(gfx::ScrollEvent& event)
{
    {
        InputMapper& mapper = m_mappers[(unsigned long)RawInput::mouse_scroll_x_pos];
        Input* mappedInput = mapper.mappedInput;
        if (mappedInput == nullptr)
        {
            assert(mappedInput->type == Input::Type::action);
            mappedInput->value = mapper.scale * event.offsetX() >= 0.0f ? event.offsetX() : 0.0f;
            mappedInput->triggered = true;
            event.process();
        }
    }
    {
        InputMapper& mapper = m_mappers[(unsigned long)RawInput::mouse_scroll_x_neg];
        Input* mappedInput = mapper.mappedInput;
        if (mappedInput == nullptr)
        {
            assert(mappedInput->type == Input::Type::action);
            mappedInput->value = mapper.scale * event.offsetX() < 0.0f ? event.offsetX() : 0.0f;
            mappedInput->triggered = true;
            event.process();
        }
    }
    {
        InputMapper& mapper = m_mappers[(unsigned long)RawInput::mouse_scroll_y_pos];
        Input* mappedInput = mapper.mappedInput;
        if (mappedInput == nullptr)
        {
            assert(mappedInput->type == Input::Type::action);
            mappedInput->value = mapper.scale * event.offsetY() >= 0.0f ? event.offsetY() : 0.0f;
            mappedInput->triggered = true;
            event.process();
        }
    }
    {
        InputMapper& mapper = m_mappers[(unsigned long)RawInput::mouse_scroll_y_neg];
        Input* mappedInput = mapper.mappedInput;
        if (mappedInput == nullptr)
        {
            assert(mappedInput->type == Input::Type::action);
            mappedInput->value = mapper.scale * event.offsetY() < 0.0f ? event.offsetY() : 0.0f;
            mappedInput->triggered = true;
            event.process();
        }
    }
}

void InputContext::onMouseMoveEvent(gfx::MouseMoveEvent& event)
{
}

void InputContext::onMouseDownEvent(gfx::MouseDownEvent& event)
{
    InputMapper& mapper = m_mappers[(unsigned long)mouseCodeToRawInput(event.mouseCode())];
    Input* mappedInput = mapper.mappedInput;
    if (mappedInput == nullptr)
        return;

    if ((mappedInput->type == Input::Type::action) ||
        (mappedInput->type == Input::Type::state))
    {
        mappedInput->value = mapper.scale;
        mappedInput->triggered = true;
    }
    event.process(); 
}

void InputContext::onMouseUpEvent(gfx::MouseUpEvent& event)
{
    InputMapper& mapper = m_mappers[(unsigned long)mouseCodeToRawInput(event.mouseCode())];
    Input* mappedInput = mapper.mappedInput;
    if (mappedInput == nullptr)
        return;
    
    if (mappedInput->type == Input::Type::state)
        mappedInput->triggered = false;

    event.process();
}


}