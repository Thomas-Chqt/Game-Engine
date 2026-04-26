/*
 * ---------------------------------------------------
 * InputMapper.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef INPUTMAPPER_HPP
#define INPUTMAPPER_HPP

#include "Game-Engine/Export.hpp"
#include "Game-Engine/Event.hpp"
#include "Game-Engine/InputFwd.hpp"
#include "Game-Engine/RawInput.hpp"
#include "Game-Engine/TypeList.hpp"

#include <yaml-cpp/yaml.h>

#include <glm/glm.hpp>

#include <variant>

namespace GE
{

template<RawInput RI, InputType I> struct InputMapper;

template<>
struct GE_API InputMapper<KeyboardButton, ActionInput>
{
    using RawInputType = KeyboardButton;
    using InputType = ActionInput;

    KeyboardButton button = KeyboardButton::space;
    ActionInput* input = nullptr;

    InputMapper() = default;
    InputMapper(KeyboardButton);
    void onInputEvent(InputEvent& event);
};

template<>
struct GE_API InputMapper<KeyboardButton, StateInput>
{
    using RawInputType = KeyboardButton;
    using InputType = StateInput;

    KeyboardButton button = KeyboardButton::space;
    StateInput* input = nullptr;

    InputMapper() = default;
    InputMapper(KeyboardButton);
    void onInputEvent(InputEvent& event);
};

template<>
struct GE_API InputMapper<KeyboardButton, RangeInput>
{
    using RawInputType = KeyboardButton;
    using InputType = RangeInput;

    KeyboardButton button = KeyboardButton::space;
    float scale = 1.0f;

    RangeInput* input = nullptr;

    InputMapper() = default;
    InputMapper(KeyboardButton, float scale);
    void onInputEvent(InputEvent& event);
};

template<>
struct GE_API InputMapper<KeyboardButton, Range2DInput>
{
    using RawInputType = KeyboardButton;
    using InputType = Range2DInput;

    struct Descriptor
    {
        KeyboardButton xPos;
        KeyboardButton xNeg;
        float xScale = 1.0f;

        KeyboardButton yPos;
        KeyboardButton yNeg;
        float yScale = 1.0f;

        float triggerValue = 0.5f;
    };

    KeyboardButton xPos = KeyboardButton::d;
    KeyboardButton xNeg = KeyboardButton::a;
    float xScale = 1.0f;

    KeyboardButton yPos = KeyboardButton::w;
    KeyboardButton yNeg = KeyboardButton::s;
    float yScale = 1.0f;

    float triggerValue = 0.5f;

    bool xPosPressed = false;
    bool xNegPressed = false;
    bool yPosPressed = false;
    bool yNegPressed = false;

    Range2DInput* input = nullptr;

    InputMapper() = default;
    InputMapper(const Descriptor&);
    void onInputEvent(InputEvent& event);
};

template<typename InputT>
using KeyboardButtonInputMapper = InputMapper<KeyboardButton, InputT>;

using InputMapperTypes = InputTypes::wrapped<KeyboardButtonInputMapper>;

using VInputMapper = InputMapperTypes::into<std::variant>;

} // namespace GE

namespace YAML
{

template<GE::InputType I>
struct convert<GE::InputMapper<GE::KeyboardButton, I>>
{
    static Node encode(const GE::InputMapper<GE::KeyboardButton, I>& rhs)
    {
        Node node;
        if constexpr (std::is_same_v<I, GE::Range2DInput>)
        {
            node["xPos"] = rhs.xPos;
            node["xNeg"] = rhs.xNeg;
            node["xScale"] = rhs.xScale;
            node["yPos"] = rhs.yPos;
            node["yNeg"] = rhs.yNeg;
            node["yScale"] = rhs.yScale;
            node["triggerValue"] = rhs.triggerValue;
        }
        else
        {
            node["button"] = rhs.button;
            if constexpr (std::is_same_v<I, GE::RangeInput>)
                node["scale"] = rhs.scale;
        }
        return node;
    }

    static bool decode(const Node& node, GE::InputMapper<GE::KeyboardButton, I>& rhs)
    {
        if (!node.IsMap())
            return false;

        if constexpr (std::is_same_v<I, GE::Range2DInput>)
        {
            if (!node["xPos"] || !node["xNeg"] || !node["yPos"] || !node["yNeg"])
                return false;

            rhs = GE::InputMapper<GE::KeyboardButton, GE::Range2DInput>(GE::InputMapper<GE::KeyboardButton, GE::Range2DInput>::Descriptor{
                .xPos         = node["xPos"].as<GE::KeyboardButton>(),
                .xNeg         = node["xNeg"].as<GE::KeyboardButton>(),
                .xScale       = node["xScale"] ? node["xScale"].as<float>() : 1.0f,
                .yPos         = node["yPos"].as<GE::KeyboardButton>(),
                .yNeg         = node["yNeg"].as<GE::KeyboardButton>(),
                .yScale       = node["yScale"] ? node["yScale"].as<float>() : 1.0f,
                .triggerValue = node["triggerValue"] ? node["triggerValue"].as<float>() : 0.5f
            });
        }
        else
        {
            if (!node["button"])
                return false;

            if constexpr (std::is_same_v<I, GE::RangeInput>)
                rhs = GE::InputMapper<GE::KeyboardButton, I>(
                    node["button"].as<GE::KeyboardButton>(),
                    node["scale"] ? node["scale"].as<float>() : 1.0f
                );
            else
                rhs = GE::InputMapper<GE::KeyboardButton, I>(node["button"].as<GE::KeyboardButton>());
        }

        return true;
    }
};

} // namespace YAML

#endif
