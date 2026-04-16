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
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <concepts>

namespace GE
{

template<typename T>
struct InputCallback
{
    using type = std::function<void(const T&)>;
};

template<>
struct InputCallback<void>
{
    using type = std::function<void()>;
};

template<typename T>
struct InputValue
{
    using type = T;
};

template<>
struct InputValue<void>
{
    using type = std::monostate;
};

template<InputClass Class, typename T>
struct Input
{
    using CallbackType = typename InputCallback<T>::type;
    using ValueType = typename InputValue<T>::type;

    CallbackType callback;
    ValueType value;

    std::optional<VInputMapper> mapper;
    bool triggered = false;

    template<RawInput RI, typename... Args>
    void setMapper(Args&&... args) requires std::constructible_from<InputMapper<RI, Input<Class, T>>, Args...>
    {
        using Mapper = InputMapper<RI, Input<Class, T>>;

        mapper = Mapper(std::forward<Args>(args)...);
        std::get<Mapper>(*mapper).input = this;
    }

    void dispatch()
    {
        if (triggered)
        {
            if constexpr (std::is_void_v<T>)
                callback();
            else
                callback(value);
        }
        if constexpr (Class == InputClass::action)
            triggered = false;
    }

    Input() = default;
    Input(const Input& other)
        : callback(other.callback)
        , value(other.value)
        , mapper(other.mapper)
        , triggered(other.triggered)
    {
        if (!mapper)
            return;
        std::visit([&](auto& m)
        {
            if constexpr (std::is_same_v<typename std::remove_cvref_t<decltype(m)>::InputType, Input<Class, T>>)
                m.input = this;
        },
        *mapper);
    }
};

using VInput = std::variant<ActionInput, StateInput, RangeInput, Range2DInput>;

}

#endif
