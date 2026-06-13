#include "convert_asset.hpp"
#include "Game-Engine/AssetLocation.hpp"

#include <Game-Engine/AssetManager.hpp>

#include <yaml-cpp/yaml.h>


namespace YAML
{

template<GE::ManagableAsset T>
struct convert<GE::AssetLocation<T>>
{
    static Node encode(const GE::AssetLocation<T>& rhs)
    {
        Node node;
        node["containerPath"] = rhs.containerPath.string();
        node["index"] = rhs.index;
        return node;
    }

    static bool decode(const Node& node, GE::AssetLocation<T>& rhs)
    {
        if (!node.IsMap() || !node["containerPath"])
            return false;

        rhs.containerPath = std::filesystem::path(node["containerPath"].as<std::string>());
        rhs.index = node["index"] ? node["index"].as<uint32_t>() : 0;
        return true;
    }
};

template<>
struct convert<GE::VAssetLocation>
{
    static Node encode(const GE::VAssetLocation& rhs)
    {
        Node node;
        std::visit([&]<typename T>(const GE::AssetLocation<T>& location) {
            node["type"] = std::string(GE::ManagableAssetTraits<T>::name);
            node["data"] = location;
        }, rhs);
        return node;
    }

    static bool decode(const Node& node, GE::VAssetLocation& rhs)
    {
        if (!node.IsMap() || !node["type"] || !node["data"])
            return false;

        const auto type = node["type"].as<std::string>();
        return GE::anyOfType<GE::ManagableAssetTypes>([&]<typename AssetT>() {
            if (type != GE::ManagableAssetTraits<AssetT>::name)
                return false;
            rhs = node["data"].as<GE::AssetLocation<AssetT>>();
            return true;
        });
    }
};

Node convert<std::tuple<std::string, GE::VAssetLocation, GE::AssetID, std::vector<GE::AssetID>>>::encode(const std::tuple<std::string, GE::VAssetLocation, GE::AssetID, std::vector<GE::AssetID>>& rhs)
{
    Node node;
    auto& [name, location, id, dependentAssets] = rhs;
    node["name"] = name;
    node["location"] = location;
    node["id"] = id;
    node["dependentAssets"] = dependentAssets;
    return node;
}

bool convert<std::tuple<std::string, GE::VAssetLocation, GE::AssetID, std::vector<GE::AssetID>>>::decode(const Node& node, std::tuple<std::string, GE::VAssetLocation, GE::AssetID, std::vector<GE::AssetID>>& rhs)
{
    if (!node.IsMap() || !node["name"] || !node["location"] || !node["id"])
        return false;

    rhs = {
        node["name"].as<std::string>(),
        node["location"].as<GE::VAssetLocation>(),
        node["id"].as<GE::AssetID>(),
        node["dependentAssets"] ? node["dependentAssets"].as<std::vector<GE::AssetID>>() : std::vector<GE::AssetID>{}
    };
    return true;
}

}
