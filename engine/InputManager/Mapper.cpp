/*
 * ---------------------------------------------------
 * Mapper.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/24 15:54:54
 * ---------------------------------------------------
 */

#include "InputManager/Mapper.hpp"
#include "InputManager/Input.hpp"
#include "InputManager/RawInput.hpp"
#include "Graphics/Event.hpp"
#include "Math/Vector.hpp"
#include "UtilsCPP/Func.hpp"
#include <cassert>

namespace GE
{

// KeyboardButton, ActionInput

Mapper<KeyboardButton, ActionInput>::Mapper(KeyboardButton btn, ActionInput& ipt)
    : m_button(btn), m_input(ipt)
{
}

utils::UniquePtr<IMapper> Mapper<KeyboardButton, ActionInput>::clone(Input* ipt) const
{
    ActionInput* actionIpt = dynamic_cast<ActionInput*>(ipt);
    assert(actionIpt != nullptr);
    auto output = utils::makeUnique<Mapper<KeyboardButton, ActionInput>>(m_button, *actionIpt);
    return output.staticCast<IMapper>();
}

void Mapper<KeyboardButton, ActionInput>::onInputEvent(gfx::InputEvent& event)
{
    event.dispatch(utils::Func<void(gfx::KeyDownEvent&)>(*this, &Mapper<KeyboardButton, ActionInput>::onKeyDownEvent));
}

void Mapper<KeyboardButton, ActionInput>::onKeyDownEvent(gfx::KeyDownEvent& keyDownEvent)
{
    if (keyDownEvent.keyCode() == (int)m_button && keyDownEvent.isRepeat() == false)
    {
        m_input.triggered = true;
        keyDownEvent.markAsProcessed();
    }
}

// KeyboardButton, StateInput

Mapper<KeyboardButton, StateInput>::Mapper(KeyboardButton btn, StateInput& ipt)
    : m_button(btn), m_input(ipt)
{
}

utils::UniquePtr<IMapper> Mapper<KeyboardButton, StateInput>::clone(Input* ipt) const
{
    StateInput* stateIpt = dynamic_cast<StateInput*>(ipt);
    assert(stateIpt != nullptr);
    auto output = utils::makeUnique<Mapper<KeyboardButton, StateInput>>(m_button, *stateIpt);
    return output.staticCast<IMapper>();
}

void Mapper<KeyboardButton, StateInput>::onInputEvent(gfx::InputEvent& event)
{
    if (event.dispatch(utils::Func<void(gfx::KeyDownEvent&)>(*this, &Mapper<KeyboardButton, StateInput>::onKeyDownEvent))) return;
    if (event.dispatch(utils::Func<void(gfx::KeyUpEvent&)>(*this, &Mapper<KeyboardButton, StateInput>::onKeyUpEvent))) return;
}

void Mapper<KeyboardButton, StateInput>::onKeyDownEvent(gfx::KeyDownEvent& keyDownEvent)
{
    if (keyDownEvent.keyCode() == (int)m_button && keyDownEvent.isRepeat() == false)
    {
        m_input.triggered = true;
        keyDownEvent.markAsProcessed();
    }
}

void Mapper<KeyboardButton, StateInput>::onKeyUpEvent(gfx::KeyUpEvent& keyUpEvent)
{
    if (keyUpEvent.keyCode() == (int)m_button)
    {
        m_input.triggered = false;
        keyUpEvent.markAsProcessed();
    }
}

// KeyboardButton, RangeInput

Mapper<KeyboardButton, RangeInput>::Mapper(const Descriptor& btn, RangeInput& ipt)
    : m_button(btn.button), m_scale(btn.scale), m_input(ipt)
{
}

utils::UniquePtr<IMapper> Mapper<KeyboardButton, RangeInput>::clone(Input* ipt) const
{
    RangeInput* rangeIpt = dynamic_cast<RangeInput*>(ipt);
    assert(rangeIpt != nullptr);
    Mapper<KeyboardButton, RangeInput>::Descriptor desc;
    desc.button = m_button;
    desc.scale = m_scale;
    auto output = utils::makeUnique<Mapper<KeyboardButton, RangeInput>>(desc, *rangeIpt);
    return output.staticCast<IMapper>();
}

void Mapper<KeyboardButton, RangeInput>::onInputEvent(gfx::InputEvent& event)
{
    if (event.dispatch(utils::Func<void(gfx::KeyDownEvent&)>(*this, &Mapper<KeyboardButton, RangeInput>::onKeyDownEvent))) return;
    if (event.dispatch(utils::Func<void(gfx::KeyUpEvent&)>(*this, &Mapper<KeyboardButton, RangeInput>::onKeyUpEvent))) return;
}

void Mapper<KeyboardButton, RangeInput>::onKeyDownEvent(gfx::KeyDownEvent& keyDownEvent)
{
    if (keyDownEvent.keyCode() == (int)m_button && keyDownEvent.isRepeat() == false)
    {
        m_input.value = m_scale;
        m_input.triggered = true;
        keyDownEvent.markAsProcessed();
    }
}

void Mapper<KeyboardButton, RangeInput>::onKeyUpEvent(gfx::KeyUpEvent& keyUpEvent)
{
    if (keyUpEvent.keyCode() == (int)m_button)
    {
        m_input.triggered = true;
        keyUpEvent.markAsProcessed();
    }
}

// KeyboardButton, Range2DInput

Mapper<KeyboardButton, Range2DInput>::Mapper(const Descriptor& btn, Range2DInput& ipt)
    : m_xPos(btn.xPos), m_xNeg(btn.xNeg), m_yPos(btn.yPos), m_yNeg(btn.yNeg),
      m_scale(btn.scale),
      m_input(ipt)
{
}

utils::UniquePtr<IMapper> Mapper<KeyboardButton, Range2DInput>::clone(Input* ipt) const
{
    Range2DInput* range2dIpt = dynamic_cast<Range2DInput*>(ipt);
    assert(range2dIpt != nullptr);
    Mapper<KeyboardButton, Range2DInput>::Descriptor desc;
    desc.xPos = m_xPos;
    desc.xNeg = m_xNeg;
    desc.yPos = m_yPos;
    desc.yNeg = m_yNeg;
    desc.scale = m_scale;
    auto output = utils::makeUnique<Mapper<KeyboardButton, Range2DInput>>(desc, *range2dIpt);
    return output.staticCast<IMapper>();
}

void Mapper<KeyboardButton, Range2DInput>::onInputEvent(gfx::InputEvent& event)
{
    if (event.dispatch(utils::Func<void(gfx::KeyDownEvent&)>(*this, &Mapper<KeyboardButton, Range2DInput>::onKeyDownEvent))) return;
    if (event.dispatch(utils::Func<void(gfx::KeyUpEvent&)>(*this, &Mapper<KeyboardButton, Range2DInput>::onKeyUpEvent))) return;
}

void Mapper<KeyboardButton, Range2DInput>::onKeyDownEvent(gfx::KeyDownEvent& keyDownEvent)
{
    if (keyDownEvent.isRepeat())
        return;

    if (keyDownEvent.keyCode() == static_cast<int>(m_xPos))
        m_input.value += math::vec2f{m_scale.x, 0.0F};
    else if (keyDownEvent.keyCode() == static_cast<int>(m_xNeg))
        m_input.value -= math::vec2f{m_scale.x, 0.0F};
    else if (keyDownEvent.keyCode() == static_cast<int>(m_yPos))
        m_input.value += math::vec2f{0.0F, m_scale.y};
    else if (keyDownEvent.keyCode() == static_cast<int>(m_yNeg))
        m_input.value -= math::vec2f{0.0F, m_scale.y};
    
    if (keyDownEvent.keyCode() == static_cast<int>(m_xPos) ||
        keyDownEvent.keyCode() == static_cast<int>(m_xNeg) ||
        keyDownEvent.keyCode() == static_cast<int>(m_yPos) ||
        keyDownEvent.keyCode() == static_cast<int>(m_yNeg))
    {
        m_input.triggered = m_input.value != math::vec2f{0.0F, 0.0F};
        keyDownEvent.markAsProcessed();
    }
}

void Mapper<KeyboardButton, Range2DInput>::onKeyUpEvent(gfx::KeyUpEvent& keyUpEvent)
{
    if (keyUpEvent.keyCode() == static_cast<int>(m_xPos))
        m_input.value -= math::vec2f{m_scale.x, 0.0F};
    else if (keyUpEvent.keyCode() == static_cast<int>(m_xNeg))
        m_input.value += math::vec2f{m_scale.x, 0.0F};
    else if (keyUpEvent.keyCode() == static_cast<int>(m_yPos))
        m_input.value -= math::vec2f{0.0F, m_scale.y};
    else if (keyUpEvent.keyCode() == static_cast<int>(m_yNeg))
        m_input.value += math::vec2f{0.0F, m_scale.y};
    
    if (keyUpEvent.keyCode() == static_cast<int>(m_xPos) ||
        keyUpEvent.keyCode() == static_cast<int>(m_xNeg) ||
        keyUpEvent.keyCode() == static_cast<int>(m_yPos) ||
        keyUpEvent.keyCode() == static_cast<int>(m_yNeg))
    {
        m_input.triggered = m_input.value != math::vec2f{0.0F, 0.0F};
        keyUpEvent.markAsProcessed();
    }
}

}