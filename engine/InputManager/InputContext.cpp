/*
 * ---------------------------------------------------
 * InputContext.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/25 14:23:10
 * ---------------------------------------------------
 */

#include "InputManager/InputContext.hpp"
#include "InputManager/Mapper.hpp" // IWYU pragma: keep

namespace GE
{

void InputContext::onInputEvent(gfx::InputEvent& event)
{
    for (auto& [_, input] : m_inputs)
    {
        for (utils::uint8 i = 0; i < 2; i++)
        {
            if (input->mappers[i] == false)
                continue;

            input->mappers[i]->onInputEvent(event);

            if (event.processed())
                return;
        }
    }
}

void InputContext::dispatchInputs()
{
    for (auto& [_, input] : m_inputs) {
        if (input->triggered)
            input->dispatch();
    }
}

void InputContext::resetInputs()
{
    for (auto& [_, input] : m_inputs) {
        input->reset();
    }
}

}