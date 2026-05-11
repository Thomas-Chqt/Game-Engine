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

#include <cassert>
#include <concepts>
#include <future>
#include <map>
#include <ranges>

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
    AssetManagerView(AssetManagerView&&) noexcept;

    AssetManagerView(AssetManager*);
    AssetManagerView(AssetManager*, const std::map<AssetID, VAssetPath>& registredAssets);

    inline const std::map<AssetID, VAssetPath>& registredAssets() const
    {
        return m_registredAssets;
    }

    template<ManagableAsset T>
    AssetID registerAsset(const std::filesystem::path& path)
    {
        assert(m_assetManager);

        auto it = std::ranges::find_if(m_registredAssets, [&](const auto& pair) {
            return std::visit([](const auto& assetPath) { return assetPath.path; }, pair.second) == path;
        });
        if (it == m_registredAssets.end())
        {
            const AssetID assetId = s_nextAssetId++;
            auto [insertedIt, inserted] = m_registredAssets.try_emplace(assetId, AssetPath<T>(path));
            assert(inserted);
            m_assetManager->registerAsset(insertedIt->second);
            if (m_isLoaded)
                m_assetManager->loadAsset<T>(insertedIt->second);
            return assetId;
        }
        return it->first;
    }

    inline bool isLoaded() const
    {
        return m_isLoaded;
    }

    std::future<void> load();

    void unload();

    inline bool isAssetLoaded(AssetID assetId) const
    {
        return m_assetManager->isAssetLoaded(m_registredAssets.at(assetId));
    }

    inline bool areAssetsLoaded(AssetIdRange auto&& assetIds) const
    {
        return m_assetManager->areAssetsLoaded(assetIds | std::views::transform([&](const auto& assetId) {
            return m_registredAssets.at(assetId);
        }));
    }

    inline bool areAllAssetsLoaded() const
    {
        return m_assetManager->areAssetsLoaded(m_registredAssets | std::views::values);
    }

    inline const std::string& assetName(AssetID assetId) const
    {
        return m_assetManager->assetName(m_registredAssets.at(assetId));
    }

    template<ManagableAsset T>
    inline const std::shared_ptr<T>& getAsset(AssetID assetId) const
    {
        return m_assetManager->getAsset<T>(m_registredAssets.at(assetId));
    }

    ~AssetManagerView();

private:
    AssetManager* m_assetManager = nullptr;
    inline static AssetID s_nextAssetId = 0;

    std::map<AssetID, VAssetPath> m_registredAssets;
    bool m_isLoaded = false;

public:
    AssetManagerView& operator=(const AssetManagerView&) = delete;
    AssetManagerView& operator=(AssetManagerView&&) noexcept;
};

} // namespace GE

#endif // ASSETMANAGERVIEW_HPP
