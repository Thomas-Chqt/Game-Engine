/*
 * ---------------------------------------------------
 * RawInputs.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/19 10:14:50
 * ---------------------------------------------------
 */

#ifndef RAWINPUTS_HPP
#define RAWINPUTS_HPP

#include "UtilsCPP/Types.hpp"

namespace GE
{

enum class RawInput : utils::uint32
{
    esc_key,
    
    w_key,
    a_key,
    s_key,
    d_key,

    up_key,
    left_key,
    down_key,
    right_key,

    mouse_move_x_pos,
    mouse_move_x_neg, 
    mouse_move_y_pos,
    mouse_move_y_neg,

    mouse_scroll_x_pos,
    mouse_scroll_x_neg,
    mouse_scroll_y_pos,
    mouse_scroll_y_neg,

    count
};

}

#endif // RAWINPUTS_HPP