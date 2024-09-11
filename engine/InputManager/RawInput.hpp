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

#include "Graphics/KeyCodes.hpp"

namespace GE
{

enum class KeyboardButton : int
{
    esc        = ESC_KEY,
    one        = ONE_KEY,
    two        = TWO_KEY,
    w          = W_KEY,
    a          = A_KEY,
    s          = S_KEY,
    d          = D_KEY,
    space      = SPACE_KEY,
    up         = UP_KEY,
    left       = LEFT_KEY,
    down       = DOWN_KEY,
    right      = RIGHT_KEY,
    left_shift = LEFT_SHIFT_KEY,
};

enum class MouseButton : int
{
    l = MOUSE_L,
    r = MOUSE_R
};

}

#endif // RAWINPUT_HPP