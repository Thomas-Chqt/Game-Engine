/*
 * ---------------------------------------------------
 * AssetManagerView.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/AssetManagerView.hpp"
#include "Game-Engine/AssetManager.hpp"

#include <cassert>
#include <utility>

namespace GE
{

AssetManagerView::AssetManagerView(AssetManager* assetManager)
    : m_assetManager(assetManager)
{
    assert(m_assetManager);
}

AssetManagerView::AssetManagerView(AssetManagerView&& other) noexcept
    : m_assetManager(std::exchange(other.m_assetManager, nullptr))
    , m_assets(std::move(other.m_assets))
    , m_isLoaded(std::exchange(other.m_isLoaded, false))
{
}

AssetManager& AssetManagerView::assetManager() const
{
    assert(m_assetManager);
    return *m_assetManager;
}

const std::set<AssetID>& AssetManagerView::assets() const
{
    return m_assets;
}

void AssetManagerView::registerAssetId(AssetID assetId)
{
    assert(m_assetManager);
    assert(m_assetManager->isValidAssetId(assetId));

    if (m_isLoaded)
        m_assetManager->loadAssetBackground(assetId);
    m_assets.insert(assetId);
}

void AssetManagerView::load()
{
    assert(m_isLoaded || m_assetManager);

    if (m_isLoaded == false) {
        m_assetManager->loadAssetsBackground(m_assets);
        m_isLoaded = true;
    }
}

void AssetManagerView::unload()
{
    assert(m_isLoaded);
    assert(m_assetManager);

    m_assetManager->unloadAssets(m_assets);
    m_isLoaded = false;
}

bool AssetManagerView::isLoaded() const
{
    assert(!m_isLoaded || m_assetManager);

    return m_isLoaded && m_assetManager->areAssetsLoaded(m_assets);
}

AssetManagerView::~AssetManagerView()
{
    assert(!m_isLoaded || m_assetManager);

    if (m_isLoaded)
        unload();
}

AssetManagerView& AssetManagerView::operator=(AssetManagerView&& other) noexcept
{
    assert(this == &other || !m_isLoaded || m_assetManager);

    if (this != &other)
    {
        if (m_isLoaded)
            unload();
        m_assetManager = std::exchange(other.m_assetManager, nullptr);
        m_assets = std::move(other.m_assets);
        m_isLoaded = std::exchange(other.m_isLoaded, false);
    }
    return *this;
}

} // namespace GE
