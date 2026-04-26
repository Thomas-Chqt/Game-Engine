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

#include <glm/glm.hpp>

#include <variant>

namespace GE
{

template<RawInput RI, InputType I> struct InputMapper;

template<>
struct GE_API InputMapper<KeyboardButton, ActionInput>
{
    using InputType = ActionInput;

    KeyboardButton button;
    ActionInput* input = nullptr;

    InputMapper(KeyboardButton);
    void onInputEvent(InputEvent& event);
};

template<>
struct GE_API InputMapper<KeyboardButton, StateInput>
{
    using InputType = StateInput;

    KeyboardButton button;
    StateInput* input = nullptr;

    InputMapper(KeyboardButton);
    void onInputEvent(InputEvent& event);
};

template<>
struct GE_API InputMapper<KeyboardButton, RangeInput>
{
    using InputType = RangeInput;

    KeyboardButton button;
    float scale = 1.0f;

    RangeInput* input = nullptr;

    InputMapper(KeyboardButton, float scale);
    void onInputEvent(InputEvent& event);
};

template<>
struct GE_API InputMapper<KeyboardButton, Range2DInput>
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

    bool xPosPressed = false;
    bool xNegPressed = false;
    bool yPosPressed = false;
    bool yNegPressed = false;

    Range2DInput* input = nullptr;

    InputMapper(const Descriptor&);
    void onInputEvent(InputEvent& event);
};

template<typename InputT>
using KeyboardButtonInputMapper = InputMapper<KeyboardButton, InputT>;

using InputMapperTypes = TypeListMap_t<InputTypes, KeyboardButtonInputMapper>;

using VInputMapper = InputMapperTypes::into<std::variant>;

}

#endif
