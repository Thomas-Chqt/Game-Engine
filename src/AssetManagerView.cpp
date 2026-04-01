/*
 * ---------------------------------------------------
 * AssetManagerView.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/AssetManagerView.hpp"
#include <algorithm>
#include <ranges>

namespace GE
{

AssetManagerView::AssetManagerView(AssetManager* assetManager)
    : m_assetManager(assetManager)
{
    assert(m_assetManager);
}

AssetManagerView::AssetManagerView(AssetManager* assetManager, const std::map<AssetID, VAssetPath>& registredAssets)
    : m_assetManager(assetManager)
{
    assert(m_assetManager);
    m_assets.insert_range(registredAssets | std::views::filter([](auto& element){ return element.first != BUILT_IN_CUBE_ASSET_ID; }));
    s_nextAssetId = std::ranges::max(m_assets | std::views::transform([](auto& element){ return element.first; })) + 1;
}

void AssetManagerView::unloadAsset(AssetID assetId)
{
    assert(m_assetManager);
    return m_assetManager->unloadAsset(m_assets.at(assetId));
}

AssetManagerView::~AssetManagerView()
{
    assert(m_assetManager);
    for (auto& [id, vAssetPath] : m_assets) {
        m_assetManager->unloadAsset(vAssetPath);
    }
}

}
