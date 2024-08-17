/*
 * ---------------------------------------------------
 * InputManager.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/16 15:34:34
 * ---------------------------------------------------
 */

#include "InputManager.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/Platform.hpp"
#include "UtilsCPP/Func.hpp"
#include "Game-Engine/Inputs.hpp"

namespace GE::Inputs {
    const utils::Set<int>& pressedKeys() { return GE::InputManager::shared().pressedKeys(); }
}

namespace GE
{

InputManager::~InputManager()
{
    gfx::Platform::shared().clearCallbacks(this);
}

InputManager::InputManager()
{
    gfx::Platform::shared().addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &InputManager::onEvent), this);
}

void InputManager::onEvent(gfx::Event& event)
{
    if (event.dispatch<gfx::KeyDownEvent>(utils::Func<void(gfx::KeyDownEvent&)>(*this, &InputManager::onKeyDownEvent)))
        return;
    if (event.dispatch<gfx::KeyUpEvent>(utils::Func<void(gfx::KeyUpEvent&)>(*this, &InputManager::onKeyUpEvent)))
        return;
    if (event.dispatch<gfx::ScrollEvent>(utils::Func<void(gfx::ScrollEvent&)>(*this, &InputManager::onScrollEvent)))
        return;
    if (event.dispatch<gfx::MouseMoveEvent>(utils::Func<void(gfx::MouseMoveEvent&)>(*this, &InputManager::onMouseMoveEvent)))
        return;
    if (event.dispatch<gfx::MouseDownEvent>(utils::Func<void(gfx::MouseDownEvent&)>(*this, &InputManager::onMouseDownEvent)))
        return;
    if (event.dispatch<gfx::MouseUpEvent>(utils::Func<void(gfx::MouseUpEvent&)>(*this, &InputManager::onMouseUpEvent)))
        return;
}

void InputManager::onKeyDownEvent(gfx::KeyDownEvent& event)
{
    if (event.isRepeat() == false)
        m_pressedKeys.insert(event.keyCode());
}

void InputManager::onKeyUpEvent(gfx::KeyUpEvent& event)
{
    m_pressedKeys.remove(m_pressedKeys.find(event.keyCode()));
}

void InputManager::onScrollEvent(gfx::ScrollEvent& event)
{
}

void InputManager::onMouseMoveEvent(gfx::MouseMoveEvent& event)
{
}

void InputManager::onMouseDownEvent(gfx::MouseDownEvent& event)
{
    m_pressedMouseButtons.insert(event.mouseCode());
}

void InputManager::onMouseUpEvent(gfx::MouseUpEvent& event)
{
    m_pressedMouseButtons.remove(m_pressedMouseButtons.find(event.mouseCode()));
}

}