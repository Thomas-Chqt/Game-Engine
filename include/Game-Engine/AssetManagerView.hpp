/*
 * ---------------------------------------------------
 * AssetManagerView.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef ASSETMANAGERVIEW_HPP
#define ASSETMANAGERVIEW_HPP

#include "Game-Engine/AssetLocation.hpp"
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

class GE_API AssetManagerView
{
public:
    AssetManagerView() = delete;
    AssetManagerView(const AssetManagerView&) = delete;
    AssetManagerView(AssetManagerView&&) noexcept;

    AssetManagerView(AssetManager*);

    AssetManagerView(AssetManager*, const std::ranges::range auto& registeredAssets) requires (std::same_as<std::ranges::range_value_t<std::remove_cvref_t<decltype(registeredAssets)>>, std::pair<std::optional<VAssetLocation>, AssetID>>);

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

AssetManagerView::AssetManagerView(AssetManager* assetManager, const std::ranges::range auto& registeredAssets) requires (std::same_as<std::ranges::range_value_t<std::remove_cvref_t<decltype(registeredAssets)>>, std::pair<std::optional<VAssetLocation>, AssetID>>)
    : m_assetManager(assetManager)
{
    assert(m_assetManager);
    for (auto& [vAssetLocation, assetId] : registeredAssets) {
        assert(vAssetLocation.has_value() || m_assetManager->isValidAssetId(assetId));
        if (vAssetLocation.has_value()) {
            std::visit([&]<ManagableAsset T>(const AssetLocation<T>& assetLocation) {
                const AssetID registeredAssetId = m_assetManager->registerAsset<T>(assetLocation.containerPath.stem().string(), assetLocation.containerPath, assetLocation.index, assetId);
                assert(registeredAssetId == assetId);
                m_assets.insert(registeredAssetId);
            }, vAssetLocation.value());
        }
        else
            registerAssetId(assetId);
    }
}

template<ManagableAsset T>
AssetID AssetManagerView::registerAsset(const std::filesystem::path& path)
{
    assert(m_assetManager);
    AssetID assetId = m_assetManager->registerAsset<T>(path.stem().string(), path, 0);
    registerAssetId(assetId);
    return assetId;
}

} // namespace GE

#endif // ASSETMANAGERVIEW_HPP
