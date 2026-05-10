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

#include <algorithm>
#include <cassert>
#include <concepts>
#include <future>
#include <map>
#include <ranges>
#include <variant>

namespace GE
{

using AssetID = uint64_t;

template<typename T>
concept AssetIdRange = std::ranges::range<T> && std::convertible_to<std::ranges::range_value_t<T>, AssetID>;

class GE_API AssetManagerView
{
public:
    AssetManagerView() = delete;
    AssetManagerView(const AssetManagerView&) = delete;
    AssetManagerView(AssetManagerView&&) = default;

    AssetManagerView(AssetManager*);
    AssetManagerView(AssetManager*, const std::map<AssetID, VAssetPath>& registredAssets);

    inline const std::map<AssetID, VAssetPath>& registredAssets() const { return m_registredAssets; }

    template<ManagableAsset T>
    AssetID registerAsset(const std::filesystem::path& path)
    {
        assert(m_assetManager);
        auto it = std::ranges::find_if(m_registredAssets, [&](auto pair){ return std::visit([](const auto& path){ return path.path; }, pair.second) == path; });
        if (it == m_registredAssets.end())
        {
            const AssetID assetId = s_nextAssetId++;
            auto [_, inserted] = m_registredAssets.try_emplace(assetId, AssetPath<T>(path));
            assert(inserted);
            m_assetManager->registerAsset(AssetPath<T>(path));
            return assetId;
        }
        return it->first;
    }

    template<ManagableAsset T>
    const std::shared_future<const std::shared_ptr<T>&>& loadAsset(AssetID assetId) const
    {
        assert(m_assetManager);
        return m_assetManager->loadAsset<T>(m_registredAssets.at(assetId));
    }

    std::future<void> loadAssets(AssetIdRange auto&& assetIds) const
    {
        assert(m_assetManager);
        return m_assetManager->loadAssets(assetIds | std::ranges::views::transform([&](const auto& assetId) { return m_registredAssets.at(assetId); }));
    }

    inline std::future<void> loadAllAssets() const { return loadAssets(m_registredAssets | std::views::transform([](const auto& asset) { return asset.first; })); }

    inline bool isAssetLoaded(AssetID assetId) const
    {
        assert(m_assetManager);
        return m_assetManager->isAssetLoaded(m_registredAssets.at(assetId));
    }

    bool areAssetsLoaded(AssetIdRange auto&& assetIds) const
    {
        assert(m_assetManager);
        return m_assetManager->areAssetsLoaded(assetIds | std::views::transform([&](const auto& assetId) { return m_registredAssets.at(assetId); }));
    }

    inline bool areAllAssetsLoaded() const { return areAssetsLoaded(m_registredAssets | std::views::transform([](const auto& asset) { return asset.first; })); }

    void unloadAssets(AssetIdRange auto&& assetIds)
    {
        assert(m_assetManager);
        m_assetManager->unloadAssets(assetIds | std::views::transform([&](const auto& assetId) { return m_registredAssets.at(assetId); }));
    }

    void unloadAsset(AssetID);
    void unloadAllAssets();

    const std::string& assetName(AssetID assetId) const;

    template<ManagableAsset T>
    inline const std::shared_ptr<T>& getAsset(AssetID assetId) { return loadAsset<T>(assetId).get(); }

    ~AssetManagerView();

private:
    AssetManager* m_assetManager = nullptr;
    inline static AssetID s_nextAssetId = 0;

    std::map<AssetID, VAssetPath> m_registredAssets;

public:
    AssetManagerView& operator=(const AssetManagerView&) = delete;
    AssetManagerView& operator=(AssetManagerView&&) = default;
};

} // namespace GE

#endif // ASSETMANAGERVIEW_HPP
