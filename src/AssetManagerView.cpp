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
#include <future>
#include <ranges>
#include <utility>

namespace GE
{

AssetManagerView::AssetManagerView(AssetManager* assetManager)
    : m_assetManager(assetManager)
{
    assert(m_assetManager);
}

AssetManagerView::AssetManagerView(AssetManagerView&& other) noexcept
    : m_assetManager(other.m_assetManager) // m_assetManager stay not null so `other` stay in a valid state for the descructor
    , m_registredAssets(std::move(other.m_registredAssets))
    , m_isLoaded(std::exchange(other.m_isLoaded, false))
{
}

AssetManagerView::AssetManagerView(AssetManager* assetManager, const std::map<AssetID, VAssetPath>& registredAssets)
    : m_assetManager(assetManager)
    , m_registredAssets(registredAssets)
{
    assert(m_assetManager);
    for (const auto& [assetId, vAssetPath] : m_registredAssets)
    {
        m_assetManager->registerAsset(vAssetPath);
        s_nextAssetId = std::max(s_nextAssetId, assetId + 1);
    }
}

std::future<void> AssetManagerView::load()
{
    assert(m_assetManager);
    if (m_isLoaded)
        return std::async(std::launch::deferred, [] {});

    std::future<void> assetsFuture = m_assetManager->loadAssets(m_registredAssets | std::views::values);
    m_isLoaded = true;
    return assetsFuture;
}

void AssetManagerView::unload()
{
    assert(m_assetManager);

    if (!m_isLoaded)
        return;

    m_assetManager->unloadAssets(m_registredAssets | std::views::values);
    m_isLoaded = false;
}

AssetManagerView::~AssetManagerView()
{
    unload();
}

AssetManagerView& AssetManagerView::operator=(AssetManagerView&& other) noexcept
{
    if (this != &other)
    {
        unload();
        m_assetManager = other.m_assetManager; // m_assetManager stay not null so `other` stay in a valid state for the descructor
        m_registredAssets = std::move(other.m_registredAssets);
        m_isLoaded = std::exchange(other.m_isLoaded, false);
    }
    return *this;
}

} // namespace GE
