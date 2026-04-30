/*
 * ---------------------------------------------------
 * AssetManagerView.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/AssetManagerView.hpp"

#include <algorithm>
#include <cassert>
#include <utility>

namespace GE
{

AssetManagerView::AssetManagerView(AssetManager* assetManager)
    : m_assetManager(assetManager)
{
    assert(m_assetManager);
}

AssetManagerView::AssetManagerView(AssetManager* assetManager, const std::map<VAssetPath, AssetID>& registredAssets)
    : m_assetManager(assetManager)
    , m_registredAssets(registredAssets)
{
    assert(m_assetManager);
    for (const auto& [vAssetPath, assetID] : m_registredAssets)
    {
        if (assetID != BUILT_IN_CUBE_ASSET_ID)
            m_assetManager->registerAsset(vAssetPath);
        auto [it, inserted] = m_assets.insert(std::make_pair(assetID, vAssetPath));
        assert(inserted);
        s_nextAssetId = std::max(s_nextAssetId, assetID + 1);
    }
}

void AssetManagerView::unloadAsset(AssetID assetId)
{
    assert(m_assetManager);
    if (assetId == BUILT_IN_CUBE_ASSET_ID)
        return m_assetManager->unloadBuiltInCube();
    return m_assetManager->unloadAsset(m_assets.at(assetId));
}

void AssetManagerView::unloadAllAssets()
{
    assert(m_assetManager);
    unloadAssets(m_assets | std::views::transform([](const auto& asset) { return asset.first; }));
}

AssetManagerView::~AssetManagerView()
{
    unloadAllAssets();
}

}
