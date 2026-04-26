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
#include "Game-Engine/TypeList.hpp"

#include <yaml-cpp/yaml.h>

#include <array>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

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

using RawInputTypes = TypeList<KeyboardButton, MouseButton>;

inline constexpr std::array<KeyboardButton, 13> KEYBOARD_BUTTONS = {
    KeyboardButton::esc,
    KeyboardButton::one,
    KeyboardButton::two,
    KeyboardButton::w,
    KeyboardButton::a,
    KeyboardButton::s,
    KeyboardButton::d,
    KeyboardButton::space,
    KeyboardButton::up,
    KeyboardButton::left,
    KeyboardButton::down,
    KeyboardButton::right,
    KeyboardButton::left_shift
};

constexpr std::string_view keyboardButtonName(KeyboardButton button)
{
    switch (button)
    {
        case KeyboardButton::esc:        return "esc";
        case KeyboardButton::one:        return "1";
        case KeyboardButton::two:        return "2";
        case KeyboardButton::w:          return "w";
        case KeyboardButton::a:          return "a";
        case KeyboardButton::s:          return "s";
        case KeyboardButton::d:          return "d";
        case KeyboardButton::space:      return "space";
        case KeyboardButton::up:         return "up";
        case KeyboardButton::left:       return "left";
        case KeyboardButton::down:       return "down";
        case KeyboardButton::right:      return "right";
        case KeyboardButton::left_shift: return "left_shift";
    }
    std::unreachable();
}

inline std::optional<KeyboardButton> keyboardButtonFromName(std::string_view name)
{
    for (KeyboardButton button : KEYBOARD_BUTTONS)
    {
        if (keyboardButtonName(button) == name)
            return button;
    }
    return std::nullopt;
}

template<typename T>
concept RawInput = IsTypeInList<std::remove_cvref_t<T>, RawInputTypes>;

template<RawInput T> struct RawInputTraits;

template<> struct RawInputTraits<KeyboardButton> { static constexpr std::string_view name = "KeyboardButton"; };
template<> struct RawInputTraits<MouseButton>    { static constexpr std::string_view name = "MouseButton";    };

} // namespace GE

namespace YAML
{

template<>
struct convert<GE::KeyboardButton>
{
    static Node encode(const GE::KeyboardButton& rhs)
    {
        return Node(std::string(GE::keyboardButtonName(rhs)));
    }

    static bool decode(const Node& node, GE::KeyboardButton& rhs)
    {
        if (!node.IsScalar())
            return false;

        if (const std::optional<GE::KeyboardButton> button = GE::keyboardButtonFromName(node.as<std::string>()))
            return rhs = *button, true;
        return false;
    }
};

}

#endif // RAWINPUT_HPP
