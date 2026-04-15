/*
 * ---------------------------------------------------
 * Input.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef INPUT_HPP
#define INPUT_HPP

#include "Game-Engine/InputFwd.hpp"
#include "Game-Engine/InputMapper.hpp"
#include "Game-Engine/RawInput.hpp"

#include <functional>
#include <utility>
#include <variant>
#include <concepts>

namespace GE
{

template<InputClass Class, typename T>
struct Input
{
    std::function<void(const T&)> callback;
    T value;

    VInputMapper mapper;
    bool triggered = false;

    template<RawInput RI, typename... Args>
    void setMapper(Args&&... args) requires std::constructible_from<InputMapper<RI, Input<Class, T>>, Args...>
    {
        using Mapper = InputMapper<RI, Input<Class, T>>;

        mapper = Mapper(std::forward<Args>(args)...);
        std::get<Mapper>(mapper).input = this;
    }

    void dispatch()
    {
        if (triggered)
            callback(value);
        if constexpr (Class == InputClass::action)
            triggered = false;
    }
};

template<InputClass Class>
struct Input<Class, void>
{
    std::function<void()> callback;

    VInputMapper mapper;
    bool triggered = false;

    template<RawInput RI, typename... Args>
    void setMapper(Args&&... args) requires std::constructible_from<InputMapper<RI, Input<Class, void>>, Args...>
    {
        using Mapper = InputMapper<RI, Input<Class, void>>;

        mapper = Mapper(std::forward<Args>(args)...);
        std::get<Mapper>(mapper).input = this;
    }

    void dispatch()
    {
        if (triggered)
            callback();
        if constexpr (Class == InputClass::action)
            triggered = false;
    }
};

using VInput = std::variant<ActionInput, StateInput, RangeInput, Range2DInput>;

}

#endif
