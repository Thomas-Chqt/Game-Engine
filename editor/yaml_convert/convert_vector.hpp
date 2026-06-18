#pragma once

#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace YAML
{

template<>
struct convert<glm::vec2>
{
    static Node encode(const glm::vec2& rhs);
    static bool decode(const Node& node, glm::vec2& rhs);
};

template<>
struct convert<glm::vec3>
{
    static Node encode(const glm::vec3& rhs);
    static bool decode(const Node& node, glm::vec3& rhs);
};

template<>
struct convert<glm::quat>
{
    static Node encode(const glm::quat& rhs);
    static bool decode(const Node& node, glm::quat& rhs);
};

}
