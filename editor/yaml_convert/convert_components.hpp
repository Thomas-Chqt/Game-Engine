#pragma once

#include <Game-Engine/Components.hpp>

#include <yaml-cpp/yaml.h>

namespace YAML
{


template<>
struct convert<std::pair<GE::EntityID, std::vector<GE::ComponentVariant>>>
{
    static Node encode(const std::pair<GE::EntityID, std::vector<GE::ComponentVariant>>& rhs);
    static bool decode(const Node& node, std::pair<GE::EntityID, std::vector<GE::ComponentVariant>>& rhs);
};

}
