#pragma once

#include "Game-Engine/TypeList.hpp"

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <type_traits>
#include <variant>
#include <string_view>

namespace GE
{

using ScriptValueTypes = TypeList<bool, int64_t, float, glm::vec2, glm::vec3, std::string>;
using VScriptValue = ScriptValueTypes::into<std::variant>;

template<typename T>
concept ScriptValueType = IsTypeInList<std::remove_cvref_t<T>, ScriptValueTypes>;

template<ScriptValueType T>
struct ScriptValueTraits
{
    static_assert(false);
};

template<> struct ScriptValueTraits<bool>        { static constexpr std::string_view name = "bool"; };
template<> struct ScriptValueTraits<int64_t>     { static constexpr std::string_view name = "int"; };
template<> struct ScriptValueTraits<float>       { static constexpr std::string_view name = "float"; };
template<> struct ScriptValueTraits<glm::vec2>   { static constexpr std::string_view name = "vec2"; };
template<> struct ScriptValueTraits<glm::vec3>   { static constexpr std::string_view name = "vec3"; };
template<> struct ScriptValueTraits<std::string> { static constexpr std::string_view name = "string"; };

}
