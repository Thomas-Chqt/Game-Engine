/*
 * ---------------------------------------------------
 * AssetManager.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/13 12:00:28
 * ---------------------------------------------------
 */

#include "Game-Engine/AssetManager.hpp"

#include <Graphics/Device.hpp>

namespace GE
{

AssetManager::AssetManager(gfx::Device* device)
    : m_device(device)
{
}

void AssetManager::unloadAsset(AssetID assetId)
{
    auto it = m_assets.find(assetId);
    assert(it != m_assets.end());

    auto visitor = [](auto&& handle) {
        handle.future.wait(); // dont need to propagate errors
        auto expected = LoadingStatus::loaded;
        if (handle.status.compare_exchange_strong(expected, LoadingStatus::unloaded))
            handle.asset.reset();
    };
    std::visit(std::move(visitor), it->second);
}

AssetManager::~AssetManager()
{
    for (auto& [id, _] : m_assets)
        unloadAsset(id);
}

Mesh AssetManager::loadMesh(const std::filesystem::path& path, gfx::CommandBuffer& commandBuffer)
{
}

} // namespace GE
