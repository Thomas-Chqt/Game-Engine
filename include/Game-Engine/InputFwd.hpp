/*
 * ---------------------------------------------------
 * InputFwd.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef INPUTFWD_HPP
#define INPUTFWD_HPP

#include "Game-Engine/TypeList.hpp"

#include <glm/glm.hpp>

#include <type_traits>

namespace GE
{

enum class InputClass
{
    action, // triggered only once
    state   // triggered every frames
};

template<InputClass Class, typename T = void>
struct Input;

template<typename T>
struct IsInput : std::false_type {};

template<InputClass Class, typename T>
struct IsInput<Input<Class, T>> : std::true_type {};

template<typename T>
concept InputType = IsInput<std::remove_cvref_t<T>>::value;

using ActionInput  = Input<InputClass::action>;
using StateInput   = Input<InputClass::state>;
using RangeInput   = Input<InputClass::state, float>;
using Range2DInput = Input<InputClass::state, glm::vec2>;

using InputTypes = TypeList<ActionInput, StateInput, RangeInput, Range2DInput>;

}

#endif
