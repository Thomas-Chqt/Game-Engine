/*
 * ---------------------------------------------------
 * InputContext.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/InputContext.hpp"

#include <cassert>
#include <optional>
#include <utility>

namespace GE
{

void InputContext::addInput(const std::string& name, const VInput& input)
{
    auto [_, insterted] = m_inputs.insert(std::make_pair(name, input));
    assert(insterted);
}

void InputContext::onInputEvent(InputEvent& event)
{
    for (auto& [_, vInput] : m_inputs)
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
    for (auto& [_, vInput] : m_inputs)
        std::visit([](auto& input) { input.dispatch(); }, vInput);
}

}
