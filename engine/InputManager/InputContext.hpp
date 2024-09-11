/*
 * ---------------------------------------------------
 * InputContext.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/25 13:27:42
 * ---------------------------------------------------
 */

#ifndef INPUTCONTEXT_HPP
#define INPUTCONTEXT_HPP

#include "Graphics/Event.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/String.hpp"
#include "InputManager/Input.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class InputContext
{
public:
    InputContext()                    = default;
    InputContext(const InputContext&) = delete;
    InputContext(InputContext&&)      = delete;

    template<typename T>
    T& newInput(const utils::String& name);

    template<typename T>
    inline T& getInput(const utils::String& name) { return *static_cast<T*>((Input*)m_inputs[name]); }

    inline void deleteInput(const utils::String& name) { m_inputs.remove(name); }

    void onInputEvent(gfx::InputEvent&);
    void dispatchInputs();

    inline void clear() { m_inputs.clear(); }
    
    ~InputContext() = default;

private:
    utils::Dictionary<utils::String, utils::UniquePtr<Input>> m_inputs;

public:
    InputContext& operator = (const InputContext&) = delete;
    InputContext& operator = (InputContext&&)      = delete;
};

template<typename T>
T& InputContext::newInput(const utils::String& name)
{
    auto it = m_inputs.insert(name, utils::makeUnique<T>(name).template staticCast<Input>());
    return *static_cast<T*>((Input*)it->val);
}

}

#endif // INPUTCONTEXT_HPP