#include "convert_vector.hpp"

#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

namespace YAML
{

Node convert<glm::vec2>::encode(const glm::vec2& rhs)
{
    Node node;
    node["x"] = rhs.x;
    node["y"] = rhs.y;
    return node;
}

bool convert<glm::vec2>::decode(const Node& node, glm::vec2& rhs)
{
    if (!node.IsMap() || !node["x"] || !node["y"])
        return false;

    rhs.x = node["x"].as<float>();
    rhs.y = node["y"].as<float>();
    return true;
}

Node convert<glm::vec3>::encode(const glm::vec3& rhs)
{
    Node node;
    node["x"] = rhs.x;
    node["y"] = rhs.y;
    node["z"] = rhs.z;
    return node;
}

bool convert<glm::vec3>::decode(const Node& node, glm::vec3& rhs)
{
    if (!node.IsMap() || !node["x"] || !node["y"] || !node["z"])
        return false;

    rhs.x = node["x"].as<float>();
    rhs.y = node["y"].as<float>();
    rhs.z = node["z"].as<float>();
    return true;
}

}
