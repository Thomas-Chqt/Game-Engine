/*
 * ---------------------------------------------------
 * AssetManager.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/13 12:00:28
 * ---------------------------------------------------
 */

#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Mesh.hpp"
#include "Game-Engine/MeshAssetLoader.hpp"

#include <Graphics/Device.hpp>

#include <future>
#include <chrono>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <variant>

namespace GE
{

AssetManager::AssetManager(gfx::Device* device)
    : m_device(device)
{
    assert(m_device != nullptr);
    assert(m_assetHandles.contains(BUILT_IN_CUBE_ID) == false);
    auto [_, inserted] = m_assetHandles.insert(std::make_pair(BUILT_IN_CUBE_ID, AssetHandle<Mesh>{
        .name = "build_in_cube",
        .loader = AssetLoader<Mesh>(m_device, BuiltInMesh::cube)
    }));
    assert(inserted);
}

std::future<void> AssetManager::loadAsset(AssetID assetId)
{
    assert(m_assetHandles.contains(assetId));
    VAssetHandle& vHandle = m_assetHandles.at(assetId);
    return std::visit([&](auto& handle){
        auto future = loadAssetHandle(handle);
        return std::async(std::launch::deferred, [future](){ future.get(); });
    }, vHandle);
}

void AssetManager::loadAssetBackground(AssetID assetId)
{
    assert(m_assetHandles.contains(assetId));
    VAssetHandle& vHandle = m_assetHandles.at(assetId);
    std::visit([&](auto& handle){ loadAssetHandle(handle); }, vHandle);
}

void AssetManager::unloadAsset(AssetID assetId)
{
    assert(m_assetHandles.contains(assetId));
    VAssetHandle& vHandle = m_assetHandles.at(assetId);
    std::visit([&]<typename T>(AssetHandle<T>& handle)
    {
        // is loaded
        assert(handle.loadCount > 0);
        assert(handle.future.valid());

        handle.loadCount--;
        if (handle.loadCount == 0) {
            handle.future.wait();
            handle.future = std::shared_future<std::shared_ptr<T>>();
        }
    },
    vHandle);
}

bool AssetManager::isAssetLoaded(AssetID assetId) const
{
    assert(m_assetHandles.contains(assetId));
    const VAssetHandle& vHandle = m_assetHandles.at(assetId);
    return std::visit([&]<typename T>(const AssetHandle<T>& handle) -> bool {
        return handle.future.valid() && handle.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }, vHandle);
}

const std::string& AssetManager::assetName(AssetID assetId) const
{
    assert(m_assetHandles.contains(assetId));
    const VAssetHandle& vHandle = m_assetHandles.at(assetId);
    return std::visit([&]<typename T>(const AssetHandle<T>& handle) -> const std::string& {
        return handle.name;
    }, vHandle);
}

uint32_t AssetManager::assetLoadCount(AssetID assetId) const
{
    assert(m_assetHandles.contains(assetId));
    const VAssetHandle& vHandle = m_assetHandles.at(assetId);
    return std::visit([&]<typename T>(const AssetHandle<T>& handle) -> uint32_t {
        return handle.loadCount;
    }, vHandle);
}

const std::optional<std::filesystem::path>& AssetManager::assetPath(AssetID assetId) const
{
    assert(m_assetHandles.contains(assetId));
    const VAssetHandle& vHandle = m_assetHandles.at(assetId);
    return std::visit([&]<typename T>(const AssetHandle<T>& handle) -> const std::optional<std::filesystem::path>& {
        return handle.path;
    }, vHandle);
}

bool AssetManager::isValidAssetId(AssetID assetId) const
{
    return m_assetHandles.contains(assetId);
}

} // namespace GE
