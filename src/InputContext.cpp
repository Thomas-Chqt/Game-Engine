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

void InputContext::removeInput(const std::string& name)
{
    m_inputs.erase(name);
}

bool InputContext::renameInput(const std::string& oldName, const std::string& newName)
{
    if (oldName == newName)
        return true;

    assert(newName.empty() == false);
    const VInput input = m_inputs.at(oldName);
    m_inputs.erase(oldName);
    auto [_, inserted] = m_inputs.emplace(newName, input);
    assert(inserted);
    return inserted;
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
