#pragma once

#include "yaml-cpp/yaml.h"

#include <Game-Engine/InputContext.hpp>

namespace YAML
{

template<>
struct convert<std::pair<std::string, GE::VInput>>
{
    static Node encode(const std::pair<std::string, GE::VInput>& rhs);
    static bool decode(const Node& node, std::pair<std::string, GE::VInput>& rhs);
};

}
