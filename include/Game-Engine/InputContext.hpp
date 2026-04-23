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

namespace GE
{

class GE_API InputContext
{
public:
    InputContext() = default;
    InputContext(const InputContext&) = default;
    InputContext(InputContext&&) = default;

    void addInput(const std::string& name, const VInput&);

    template<InputType T>
    void setInputCallback(const std::string& name, const T::CallbackType&);

    virtual void onInputEvent(InputEvent&);
    void dispatchInputs();

    virtual ~InputContext() = default;

private:
    std::map<std::string, VInput> m_inputs;

public:
    InputContext& operator = (const InputContext&) = default;
    InputContext& operator = (InputContext&&) = default;
};

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

}

#endif
