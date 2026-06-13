#pragma once

#include <Game-Engine/AssetManager.hpp>

#include <yaml-cpp/yaml.h>

namespace YAML
{

template<>
struct convert<std::tuple<std::string, GE::VAssetLocation, GE::AssetID, std::vector<GE::AssetID>>>
{
    static Node encode(const std::tuple<std::string, GE::VAssetLocation, GE::AssetID, std::vector<GE::AssetID>>& rhs);
    static bool decode(const Node& node, std::tuple<std::string, GE::VAssetLocation, GE::AssetID, std::vector<GE::AssetID>>& rhs);
};

}
