/*
 * ---------------------------------------------------
 * AssetManagerView.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/AssetManagerView.hpp"
#include "Game-Engine/AssetManager.hpp"

#include <algorithm>
#include <cassert>
#include <ranges>

namespace GE
{

AssetManagerView::AssetManagerView(AssetManager* assetManager)
    : m_assetManager(assetManager)
{
}

AssetManagerView::AssetManagerView(AssetManager* assetManager, const std::map<AssetID, VAssetPath>& registredAssets)
    : m_assetManager(assetManager)
    , m_registredAssets(registredAssets)
{
    assert(m_assetManager);
    for (const auto& [assetID, vAssetPath] : m_registredAssets)
    {
        m_assetManager->registerAsset(vAssetPath);
        s_nextAssetId = std::max(s_nextAssetId, assetID + 1);
    }
}

void AssetManagerView::unloadAsset(AssetID assetId)
{
    assert(m_assetManager);
    return m_assetManager->unloadAsset(m_registredAssets.at(assetId));
}

void AssetManagerView::unloadAllAssets()
{
    assert(m_assetManager);
    unloadAssets(m_registredAssets | std::views::transform([](const auto& asset) { return asset.first; }));
}

const std::string& AssetManagerView::assetName(AssetID assetId) const
{
    assert(m_assetManager);
    return m_assetManager->assetName(m_registredAssets.at(assetId));
}

AssetManagerView::~AssetManagerView()
{
    unloadAllAssets();
}

} // namespace GE
