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

#include <yaml-cpp/yaml.h>

#include <concepts>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
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

template<InputType T> struct InputTraits;

template<> struct InputTraits<ActionInput>  { static constexpr std::string_view name = "Action";  };
template<> struct InputTraits<StateInput>   { static constexpr std::string_view name = "State";   };
template<> struct InputTraits<RangeInput>   { static constexpr std::string_view name = "Range";   };
template<> struct InputTraits<Range2DInput> { static constexpr std::string_view name = "Range2D"; };

} // namespace GE

namespace YAML
{

template<>
struct convert<GE::VInput>
{
    static YAML::Node encode(const GE::VInput& rhs)
    {
        return std::visit([](const auto& input) -> YAML::Node
        {
            YAML::Node node;
            using InputType = std::remove_cvref_t<decltype(input)>;
            node["type"] = std::string(GE::InputTraits<InputType>::name);
            if (input.mapper)
            {
                Node mapperNode;
                std::visit([&](const auto& mapper)
                {
                    using MapperType = std::remove_cvref_t<decltype(mapper)>;
                    using RawInputType = typename MapperType::RawInputType;
                    mapperNode["type"] = std::string(GE::RawInputTraits<RawInputType>::name);
                    mapperNode["data"] = mapper;
                },
                *input.mapper);
                node["mapper"] = mapperNode;
            }
            return node;
        },
        rhs);
    }

    static bool decode(const YAML::Node& node, GE::VInput& rhs)
    {
        if (!node.IsMap() || !node["type"])
            return false;

        const std::string type = node["type"].as<std::string>();
        return GE::anyOfType<GE::InputTypes>([&]<typename InputType>() {
            if (type != GE::InputTraits<InputType>::name)
                return false;

            InputType input;
            if (node["mapper"])
            {
                const YAML::Node& mapperNode = node["mapper"];
                if (!mapperNode.IsMap() || !mapperNode["type"] || !mapperNode["data"])
                    return false;

                const std::string mapperType = mapperNode["type"].as<std::string>();
                const bool decoded = GE::anyOfType<GE::InputMapperTypes>([&]<typename MapperType>() {
                    if constexpr (!std::same_as<typename MapperType::InputType, InputType>)
                        return false;
                    else
                    {
                        using RawInputType = typename MapperType::RawInputType;
                        if (mapperType != GE::RawInputTraits<RawInputType>::name)
                            return false;

                        MapperType mapper = mapperNode["data"].as<MapperType>();
                        input.setMapper(mapper);
                        return true;
                    }
                });

                if (!decoded)
                    return false;
            }

            rhs = input;
            return true;
        });
    }
};

} // namespace YAML

#endif
