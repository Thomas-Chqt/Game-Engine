/*
 * ---------------------------------------------------
 * InputMapper.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef INPUTMAPPER_HPP
#define INPUTMAPPER_HPP

#include "Game-Engine/Event.hpp"
#include "Game-Engine/InputFwd.hpp"
#include "Game-Engine/RawInput.hpp"

#include <glm/glm.hpp>

#include <variant>

namespace GE
{

template<RawInput RI, typename I> struct InputMapper;

template<>
struct InputMapper<KeyboardButton, ActionInput>
{
    using InputType = ActionInput;

    KeyboardButton button;
    ActionInput* input = nullptr;

    InputMapper(KeyboardButton);
    void onInputEvent(InputEvent& event);
};

template<>
struct InputMapper<KeyboardButton, StateInput>
{
    using InputType = StateInput;

    KeyboardButton button;
    StateInput* input = nullptr;

    InputMapper(KeyboardButton);
    void onInputEvent(InputEvent& event);
};

template<>
struct InputMapper<KeyboardButton, RangeInput>
{
    using InputType = RangeInput;

    KeyboardButton button;
    float scale = 1.0f;

    RangeInput* input = nullptr;

    InputMapper(KeyboardButton, float scale);
    void onInputEvent(InputEvent& event);
};

template<>
struct InputMapper<KeyboardButton, Range2DInput>
{
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

    KeyboardButton xPos;
    KeyboardButton xNeg;
    float xScale = 1.0f;

    KeyboardButton yPos;
    KeyboardButton yNeg;
    float yScale = 1.0f;
    float triggerValue = 0.5f;
    glm::vec2 rawValue = { 0.0f, 0.0f };

    Range2DInput* input = nullptr;

    InputMapper(const Descriptor&);
    void onInputEvent(InputEvent& event);
};

using VInputMapper = std::variant<
    InputMapper<KeyboardButton, ActionInput>,
    InputMapper<KeyboardButton, StateInput>,
    InputMapper<KeyboardButton, RangeInput>,
    InputMapper<KeyboardButton, Range2DInput>
>;

}

#endif
