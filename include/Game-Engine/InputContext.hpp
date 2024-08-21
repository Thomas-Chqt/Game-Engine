/*
 * ---------------------------------------------------
 * InputContext.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/19 09:45:39
 * ---------------------------------------------------
 */

#ifndef INPUTCONTEXT_HPP
#define INPUTCONTEXT_HPP

#include "Graphics/Event.hpp"
#include "UtilsCPP/String.hpp"
#include "RawInputs.hpp"

namespace GE
{

struct Input
{
    enum class Type { action, state } type;

    bool triggered = false;
    float value = 0.0F;
    utils::Array<utils::Func<void(float)>> callbacks;

    inline Input(Type t) : type(t) {}
};

struct InputMapper
{
    Input* mappedInput = nullptr;
    float scale = 1.0F;

    inline InputMapper() {}
    inline InputMapper(Input* input) : mappedInput(input) {}
};

class InputContext
{
public:
    InputContext()                    = delete;
    InputContext(const InputContext&) = default;
    InputContext(InputContext&&)      = default;

    InputContext(const utils::String& name);

    inline utils::String name() const { return m_name; }
    bool enabled() const { return m_enabled; }

    inline void enable() { m_enabled = true; }
    inline void disable() { m_enabled = false; }

    void onEvent(gfx::Event&);

    ~InputContext() = default;

private:
    void onKeyDownEvent(gfx::KeyDownEvent&);
    void onKeyUpEvent(gfx::KeyUpEvent&);
    void onScrollEvent(gfx::ScrollEvent&);
    void onMouseMoveEvent(gfx::MouseMoveEvent&);
    void onMouseDownEvent(gfx::MouseDownEvent&);
    void onMouseUpEvent(gfx::MouseUpEvent&);


    utils::String m_name;
    bool m_enabled = true;

    InputMapper m_mappers[(unsigned long)RawInput::count];

public:
    InputContext& operator = (const InputContext&) = default;
    InputContext& operator = (InputContext&&)      = default;
};

}

#endif // INPUTCONTEXT_HPP