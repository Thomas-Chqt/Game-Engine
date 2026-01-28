/*
 * ---------------------------------------------------
 * RawInput.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/24 15:47:29
 * ---------------------------------------------------
 */

#ifndef RAWINPUT_HPP
#define RAWINPUT_HPP

#include "GLFW/glfw3.h"

namespace GE
{

enum class KeyboardButton : unsigned int
{
    esc        = GLFW_KEY_ESCAPE,
    one        = GLFW_KEY_1,
    two        = GLFW_KEY_2,
    w          = GLFW_KEY_W,
    a          = GLFW_KEY_A,
    s          = GLFW_KEY_S,
    d          = GLFW_KEY_D,
    space      = GLFW_KEY_SPACE,
    up         = GLFW_KEY_UP,
    left       = GLFW_KEY_LEFT,
    down       = GLFW_KEY_DOWN,
    right      = GLFW_KEY_RIGHT,
    left_shift = GLFW_KEY_LEFT_SHIFT,
};

enum class MouseButton : unsigned int
{
    l = GLFW_MOUSE_BUTTON_1,
    r = GLFW_MOUSE_BUTTON_2
};

}

#endif // RAWINPUT_HPP
