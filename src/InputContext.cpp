/*
 * ---------------------------------------------------
 * InputContext.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/InputContext.hpp"

#include <optional>

namespace GE
{

void InputContext::addInput(const VInput& input)
{
    m_inputs.push_back(input);
}

void InputContext::onInputEvent(InputEvent& event)
{
    for (auto& vInput : m_inputs)
    {
        std::visit([&](auto& input)
        {
            if (input.mapper == std::nullopt)
                return;
            std::visit([&](auto& mapper){ mapper.onInputEvent(event); }, *input.mapper);
        },
        vInput);
    }
}

void InputContext::dispatchInputs()
{
    for (auto& vInput : m_inputs)
        std::visit([](auto& input) { input.dispatch(); }, vInput);
}

}
