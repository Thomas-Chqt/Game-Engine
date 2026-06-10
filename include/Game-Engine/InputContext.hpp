/*
 * ---------------------------------------------------
 * InputContext.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef INPUTCONTEXT_HPP
#define INPUTCONTEXT_HPP

#include "Game-Engine/Export.hpp"
#include "Game-Engine/Event.hpp"
#include "Game-Engine/Input.hpp"

#include <map>
#include <string>
#include <type_traits>
#include <variant>

template<typename T>
concept InputRange = std::ranges::range<T> && std::convertible_to<std::ranges::range_value_t<T>, std::pair<std::string, GE::VInput>>;

namespace GE
{

class GE_API InputContext
{
public:
    InputContext() = default;
    InputContext(const InputContext&) = default;
    InputContext(InputContext&&) = default;

    InputContext(const InputRange auto&);

    void addInput(const std::string& name, const VInput&);
    void removeInput(const std::string& name);
    bool renameInput(const std::string& oldName, const std::string& newName);

    inline auto& inputs(this auto&& self) { return self.m_inputs; }

    template<InputType T>
    void setInputCallback(const std::string& name, const T::CallbackType&);

    void clearAllInputCallbacks();

    virtual void onInputEvent(InputEvent&);
    void dispatchInputs();

    virtual ~InputContext() = default;

private:
    std::map<std::string, VInput> m_inputs;

public:
    InputContext& operator = (const InputContext&) = default;
    InputContext& operator = (InputContext&&) = default;
};

InputContext::InputContext(const InputRange auto& inputs)
    : InputContext()
{
    for (auto& [name, vInput] : inputs)
        addInput(name, vInput);
}

template<InputType T>
void InputContext::setInputCallback(const std::string& name, const T::CallbackType& callback)
{
    std::visit([&](auto& input)
    {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(input)>, T>)
            input.callback = callback;
    },
    m_inputs.at(name));
}

} // namespace GE

#endif
