/*
 * ---------------------------------------------------
 * InputFwd.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef INPUTFWD_HPP
#define INPUTFWD_HPP

#include <glm/glm.hpp>

namespace GE
{

enum class InputClass
{
    action, // triggered only once
    state   // triggered every frames
};

template<InputClass Class, typename T = void>
struct Input;

using ActionInput  = Input<InputClass::action>;
using StateInput   = Input<InputClass::state>;
using RangeInput   = Input<InputClass::state, float>;
using Range2DInput = Input<InputClass::state, glm::vec2>;

}

#endif
