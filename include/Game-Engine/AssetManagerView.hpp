/*
 * ---------------------------------------------------
 * AssetManagerView.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef ASSETMANAGERVIEW_HPP
#define ASSETMANAGERVIEW_HPP

#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Export.hpp"
#include "Game-Engine/ManagableAsset.hpp"

#include <cassert>
#include <concepts>
#include <optional>
#include <ranges>
#include <set>
#include <type_traits>
#include <utility>
#include <variant>

namespace GE
{

template<ManagableAsset T>
struct AssetPath
{
    using AssetType = T;
    std::filesystem::path path;
    AssetPath(std::filesystem::path path) : path(std::move(path)) {}
    inline operator std::filesystem::path& () { return path; }
    inline operator const std::filesystem::path& () const { return path; }
};

using AssetPathTypes = ManagableAssetTypes::wrapped<AssetPath>;
using VAssetPath = AssetPathTypes::into<std::variant>;

template<typename T> struct AssetPathYamlTraits;
template<> struct AssetPathYamlTraits<AssetPath<Mesh>>         { static constexpr std::string_view name = "Mesh";    };
template<> struct AssetPathYamlTraits<AssetPath<gfx::Texture>> { static constexpr std::string_view name = "Texture"; };

class GE_API AssetManagerView
{
public:
    AssetManagerView() = delete;
    AssetManagerView(const AssetManagerView&) = delete;
    AssetManagerView(AssetManagerView&&) noexcept;

    AssetManagerView(AssetManager*);

    AssetManagerView(AssetManager*, const std::ranges::range auto& registeredAssets) requires (std::same_as<std::ranges::range_value_t<std::remove_cvref_t<decltype(registeredAssets)>>, std::pair<std::optional<VAssetPath>, AssetID>>);

    AssetManager& assetManager() const;
    const std::set<AssetID>& assets() const;

    template<ManagableAsset T>
    AssetID registerAsset(const std::filesystem::path& path);
    void registerAssetId(AssetID);

    void load();
    void unload();
    bool isLoaded() const;

    ~AssetManagerView();

private:
    AssetManager* m_assetManager = nullptr;
    std::set<AssetID> m_assets;
    // when m_loaded is true, that doesnt mean all assets are loaded, it mean that loading has
    // started and and all newlly registered asset are loaded instantly
    // to check if all assets are loaded use isLoaded()
    bool m_isLoaded = false;

public:
    AssetManagerView& operator=(const AssetManagerView&) = delete;
    AssetManagerView& operator=(AssetManagerView&&) noexcept;
};

AssetManagerView::AssetManagerView(AssetManager* assetManager, const std::ranges::range auto& registeredAssets) requires (std::same_as<std::ranges::range_value_t<std::remove_cvref_t<decltype(registeredAssets)>>, std::pair<std::optional<VAssetPath>, AssetID>>)
    : m_assetManager(assetManager)
{
    assert(m_assetManager);
    for (auto& [vAssetPath, assetId] : registeredAssets) {
        assert(vAssetPath.has_value() || m_assetManager->isValidAssetId(assetId));
        if (vAssetPath.has_value()) {
            std::visit([&]<ManagableAsset T>(const AssetPath<T>& assetPath) {
                const AssetID registeredAssetId = m_assetManager->registerAsset<T>(assetPath, assetId);
                assert(registeredAssetId == assetId);
                m_assets.insert(registeredAssetId);
            }, vAssetPath.value());
        }
        else
            registerAssetId(assetId);
    }
}

template<ManagableAsset T>
AssetID AssetManagerView::registerAsset(const std::filesystem::path& path)
{
    assert(m_assetManager);
    AssetID assetId = m_assetManager->registerAsset<T>(path);
    registerAssetId(assetId);
    return assetId;
}

} // namespace GE

namespace YAML
{

template<>
struct convert<std::pair<std::optional<GE::VAssetPath>, GE::AssetID>>
{
    static Node encode(const std::pair<std::optional<GE::VAssetPath>, GE::AssetID>& rhs)
    {
        Node node;
        node["assetId"] = rhs.second;
        if (rhs.first.has_value()) {
            Node assetNode;
            std::visit([&](const auto& assetPath)
            {
                using AssetPathT = std::remove_cvref_t<decltype(assetPath)>;
                assetNode["type"] = std::string(GE::AssetPathYamlTraits<AssetPathT>::name);
                assetNode["path"] = assetPath.path.string();
            },
            rhs.first.value());
            node["asset"] = assetNode;
        }
        return node;
    }

    static bool decode(const Node& node, std::pair<std::optional<GE::VAssetPath>, GE::AssetID>& rhs)
    {
        if (!node.IsMap() || !node["assetId"])
            return false;

        rhs.second = node["assetId"].as<GE::AssetID>();
        rhs.first.reset();

        if (!node["asset"])
            return true;

        const Node& assetNode = node["asset"];
        if (!assetNode.IsMap() || !assetNode["type"] || !assetNode["path"])
            return false;

        const auto type = assetNode["type"].as<std::string>();
        const std::filesystem::path path = assetNode["path"].as<std::string>();
        return GE::anyOfType<GE::AssetPathTypes>([&]<typename AssetPathT>() {
            if (type != GE::AssetPathYamlTraits<AssetPathT>::name)
                return false;
            rhs.first = AssetPathT(path);
            return true;
        });
    }
};

}

#endif // ASSETMANAGERVIEW_HPP
