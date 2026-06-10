#pragma once

#include "Project.hpp"

#include <yaml-cpp/yaml.h>

namespace YAML
{

template<>
struct convert<GE_Editor::Project>
{
    static Node encode(const GE_Editor::Project& rhs);
    static bool decode(const Node& node, GE_Editor::Project& rhs);
};

} // namespace YAML
