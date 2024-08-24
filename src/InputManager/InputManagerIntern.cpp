/*
 * ---------------------------------------------------
 * InputManagerIntern.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/16 15:34:34
 * ---------------------------------------------------
 */

#include "InputManager/InputManagerIntern.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/Platform.hpp"
#include "Game-Engine/Mapper.hpp" // IWYU pragma: keep
#include "UtilsCPP/Func.hpp"

namespace GE
{

void InputManagerIntern::disableGameInputs()
{
    for (auto& [_, input] : m_gameInputs)
        input->unTrigger();
    m_gameInputsEnabled = false;
}

void InputManagerIntern::disableEditorInputs()
{
    for (auto& [_, input] : m_editorInputs)
        input->unTrigger();
    m_editorInputsEnabled = false;
}

void InputManagerIntern::dispatchInputs()
{
    for (auto& [_, input] : m_editorInputs) {
        if (input->isTriggered())
            input->dispatch();
    }
    for (auto& [_, input] : m_gameInputs) {
        if (input->isTriggered())
            input->dispatch();
    }
}

InputManagerIntern::~InputManagerIntern()
{
    gfx::Platform::shared().clearCallbacks(this);
}

InputManagerIntern::InputManagerIntern()
{
    gfx::Platform::shared().addEventCallBack([&](gfx::Event& event) {
        event.dispatch(utils::Func<void(gfx::InputEvent&)>(*this, &InputManagerIntern::onInputEvent));
    }, this);
}

void InputManagerIntern::onInputEvent(gfx::InputEvent& event)
{
    if (m_editorInputsEnabled)
    {
        for (auto& [_, input] : m_editorInputs)
            input->onInputEvent(event);
    }
    if (m_gameInputsEnabled)
    {
        for (auto& [_, input] : m_gameInputs)
            input->onInputEvent(event);
    }

    if (event.dispatch<gfx::KeyDownEvent>([&](gfx::KeyDownEvent& keyDownEvent){
        if (keyDownEvent.isRepeat() == false)
            m_pressedKeys.insert(keyDownEvent.keyCode());
    })) return;

    if (event.dispatch<gfx::KeyUpEvent>([&](gfx::KeyUpEvent& keyUpEvent){
        m_pressedKeys.remove(m_pressedKeys.find(keyUpEvent.keyCode()));
    })) return;

    if (event.dispatch<gfx::MouseDownEvent>([&](gfx::MouseDownEvent& mouseDownEvent){
        m_pressedMouseButtons.insert(mouseDownEvent.mouseCode());
    })) return;

    if (event.dispatch<gfx::MouseUpEvent>([&](gfx::MouseUpEvent& mouseUpEvent){
        m_pressedMouseButtons.remove(m_pressedMouseButtons.find(mouseUpEvent.mouseCode()));
    })) return;
}

}