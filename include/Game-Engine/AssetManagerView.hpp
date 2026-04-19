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

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <future>
#include <map>
#include <ranges>

namespace GE
{

using AssetID = uint64_t;
constexpr AssetID BUILT_IN_CUBE_ASSET_ID = std::numeric_limits<uint64_t>::max();

template<typename T>
concept AssetIdRange = std::ranges::range<T> && std::convertible_to<std::ranges::range_value_t<T>, AssetID>;

class AssetManagerView
{
public:
    AssetManagerView() = delete;
    AssetManagerView(const AssetManagerView&) = delete;
    AssetManagerView(AssetManagerView&&) = default;

    AssetManagerView(AssetManager*);
    AssetManagerView(AssetManager*, const std::map<AssetID, VAssetPath>& registredAssets);

    template<ManagableAsset T>
    AssetID registerAsset(const std::filesystem::path& path)
    {
        assert(m_assetManager);
        m_assetManager->registerAsset(AssetPath<T>(path));
        auto [it, inserted] = m_assets.insert(s_nextAssetId++, AssetPath<T>(path));
        assert(inserted);
        return it->first;
    }

    template<ManagableAsset T>
    const std::shared_future<const std::shared_ptr<T>&>& loadAsset(AssetID assetId) const
    {
        assert(m_assetManager);
        if (assetId == BUILT_IN_CUBE_ASSET_ID)
            return m_assetManager->loadBuiltInCube();
        else
            return m_assetManager->loadAsset<T>(m_assets.at(assetId));
    }

    std::future<void> loadAssets(AssetIdRange auto&& assetIds) const
    {
        assert(m_assetManager);
        std::array<std::future<void>, 2> futures;
        futures[1] = m_assetManager->loadAssets(assetIds
                                                | std::views::filter([&](const auto& id) {
                                                      if (id == BUILT_IN_CUBE_ASSET_ID)
                                                          futures[0] = std::async(std::launch::deferred, [future = m_assetManager->loadBuiltInCube()] { future.get(); });
                                                      return id != BUILT_IN_CUBE_ASSET_ID;
                                                  })
                                                | std::ranges::views::transform([&](const auto& assetId) { return m_assets.at(assetId); }));

        return std::async(std::launch::deferred, [futures = std::move(futures)]() mutable {
            for (auto& f : futures)
                if (f.valid())
                    f.get();
        });
    }

    inline std::future<void> loadAllAssets() const { return loadAssets(m_assets | std::views::transform([](const auto& asset) { return asset.first; })); }

    inline bool isAssetLoaded(AssetID assetId) const
    {
        assert(m_assetManager);
        if (assetId == BUILT_IN_CUBE_ASSET_ID)
            return m_assetManager->isBuiltInCubeLoaded();
        return m_assetManager->isAssetLoaded(m_assets.at(assetId));
    }

    bool areAssetsLoaded(AssetIdRange auto&& assetIds) const
    {
        return std::ranges::all_of(assetIds, [this](const auto& assetId) {
            return isAssetLoaded(assetId);
        });
    }

    inline bool areAllAssetsLoaded() const { return areAssetsLoaded(m_assets | std::views::transform([](const auto& asset) { return asset.first; })); }
    inline const std::map<AssetID, VAssetPath>& registredAssets() const { return m_assets; }

    void unloadAssets(AssetIdRange auto&& assetIds)
    {
        assert(m_assetManager);
        m_assetManager->unloadAssets(assetIds
                                     | std::views::filter([&](const auto& id) {
                                           if (id == BUILT_IN_CUBE_ASSET_ID)
                                               m_assetManager->unloadBuiltInCube();
                                           return id != BUILT_IN_CUBE_ASSET_ID;
                                       })
                                     | std::views::transform([&](const auto& assetId) { return m_assets.at(assetId); }));
    }

    void unloadAsset(AssetID);
    void unloadAllAssets();

    template<ManagableAsset T>
    inline const std::shared_ptr<T>& getAsset(AssetID assetId) { return loadAsset<T>(assetId).get(); }

    ~AssetManagerView();

private:
    AssetManager* m_assetManager = nullptr;
    inline static AssetID s_nextAssetId = 0;

    std::map<AssetID, VAssetPath> m_assets;

public:
    AssetManagerView& operator=(const AssetManagerView&) = delete;
    AssetManagerView& operator=(AssetManagerView&&) = default;
};

} // namespace GE

#endif // ASSETMANAGERVIEW_HPP
