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

#include <concepts>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

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
    template<RawInput RI> using MapperType = InputMapper<RI, Input<Class, T>>;

    CallbackType callback;
    ValueType value;

    std::optional<VInputMapper> mapper;
    bool triggered = false;

    template<RawInput RI, typename... Args>
    void setMapper(Args&&... args) requires std::constructible_from<MapperType<RI>, Args...>
    {
        mapper = MapperType<RI>(std::forward<Args>(args)...);
        std::get<MapperType<RI>>(*mapper).input = this;
    }

    template<RawInput RI>
    void setMapper(MapperType<RI> m)
    {
        mapper = std::move(m);
        std::get<MapperType<RI>>(*mapper).input = this;
    }

    void dispatch()
    {
        if (triggered && callback)
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
        rebindMapper();
    }

    Input(Input&& other) noexcept
        : callback(std::move(other.callback))
        , value(std::move(other.value))
        , mapper(std::move(other.mapper))
        , triggered(other.triggered)
    {
        rebindMapper();
    }

    ~Input() = default;

    Input& operator=(const Input& other)
    {
        if (this == &other)
            return *this;

        callback = other.callback;
        value = other.value;
        mapper = other.mapper;
        triggered = other.triggered;
        rebindMapper();
        return *this;
    }

    Input& operator=(Input&& other) noexcept
    {
        if (this == &other)
            return *this;

        callback = std::move(other.callback);
        value = std::move(other.value);
        mapper = std::move(other.mapper);
        triggered = other.triggered;
        rebindMapper();
        return *this;
    }

private:
    void rebindMapper()
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

using VInput = InputTypes::into<std::variant>;

} // namespace GE

#endif
