/*
 * ---------------------------------------------------
 * AssetManager.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/13 12:00:28
 * ---------------------------------------------------
 */

#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/ManagableAsset.hpp"
#include "Game-Engine/Mesh.hpp"
#include "Game-Engine/MeshAssetLoader.hpp"

#include <cassert>
#include <future>
#include <mutex>
#include <string>
#include <variant>

namespace GE
{

AssetManager::AssetManager(gfx::Device* device)
    : m_device(device)
{
    assert(m_device != nullptr);

    const AssetID builtInCubeId = registerAsset<Mesh>(
        "build_in_cube",
        AssetLoader<Mesh>(m_device, BuiltInMesh::cube),
        BUILT_IN_CUBE_ID
    );
    assert(builtInCubeId == BUILT_IN_CUBE_ID);
}

std::future<void> AssetManager::loadAsset(AssetID assetId)
{
    VAssetHandle& vHandle = [&]() -> VAssetHandle& {
        std::scoped_lock lock(m_registryMutex);
        assert(m_assetHandles.contains(assetId));
        return m_assetHandles.at(assetId);
    }();

    return std::visit([&]<ManagableAsset T>(AssetHandle<T>& handle) -> std::future<void> {
        std::shared_future<std::shared_ptr<T>> future = handle.load(m_device, nullptr);
        return std::async(std::launch::deferred, [future]() { future.get(); });
    }, vHandle);
}

void AssetManager::loadAssetDetached(AssetID assetId)
{
    VAssetHandle& vHandle = [&]() -> VAssetHandle& {
        std::scoped_lock lock(m_registryMutex);
        assert(m_assetHandles.contains(assetId));
        return m_assetHandles.at(assetId);
    }();

    std::visit([&]<ManagableAsset T>(AssetHandle<T>& handle) {
        handle.load(m_device, nullptr);
    }, vHandle);
}

void AssetManager::unloadAsset(AssetID assetId)
{
    VAssetHandle& vHandle = [&]() -> VAssetHandle& {
        std::scoped_lock lock(m_registryMutex);
        assert(m_assetHandles.contains(assetId));
        return m_assetHandles.at(assetId);
    }();

    std::visit([&]<ManagableAsset T>(AssetHandle<T>& handle) {
        handle.unload();
    }, vHandle);
}

std::set<AssetID> AssetManager::assetIds() const
{
    std::lock_guard lock(m_registryMutex);
    std::set<AssetID> assetIds;
    for (const auto& [assetId, _] : m_assetHandles)
        assetIds.insert(assetId);
    return assetIds;
}

AssetID AssetManager::assetId(const std::filesystem::path& path) const
{
    std::scoped_lock lock(m_registryMutex);
    assert(m_registeredAssets.contains(path));
    return m_registeredAssets.at(path);
}

std::string AssetManager::assetName(AssetID assetId) const
{
    std::scoped_lock lock(m_registryMutex);
    assert(m_assetHandles.contains(assetId));
    const VAssetHandle& vHandle = m_assetHandles.at(assetId);
    return std::visit([]<ManagableAsset T>(const AssetHandle<T>& handle) -> std::string {
        return handle.name();
    }, vHandle);
}

std::optional<std::filesystem::path> AssetManager::assetPath(AssetID assetId) const
{
    std::scoped_lock lock(m_registryMutex);
    assert(m_assetHandles.contains(assetId));
    const VAssetHandle& vHandle = m_assetHandles.at(assetId);
    return std::visit([]<ManagableAsset T>(const AssetHandle<T>& handle) -> std::optional<std::filesystem::path> {
        return handle.path();
    }, vHandle);
}

uint32_t AssetManager::assetLoadCount(AssetID assetId) const
{
    const VAssetHandle& vHandle = [&]() -> const VAssetHandle& {
        std::scoped_lock lock(m_registryMutex);
        assert(m_assetHandles.contains(assetId));
        return m_assetHandles.at(assetId);
    }();

    return std::visit([]<ManagableAsset T>(const AssetHandle<T>& handle) {
        return handle.loadCount();
    }, vHandle);
}

bool AssetManager::isRegistered(const std::filesystem::path& path) const
{
    std::lock_guard lock(m_registryMutex);
    return m_registeredAssets.contains(path);
}

bool AssetManager::isValidAssetId(AssetID assetId) const
{
    std::lock_guard lock(m_registryMutex);
    return m_assetHandles.contains(assetId);
}

bool AssetManager::isAssetLoaded(AssetID assetId) const
{
    std::lock_guard lock(m_registryMutex);
    assert(m_assetHandles.contains(assetId));
    const VAssetHandle& vHandle = m_assetHandles.at(assetId);
    return std::visit([]<ManagableAsset T>(const AssetHandle<T>& handle) -> bool {
        return handle.isLoaded();
    }, vHandle);
}

} // namespace GE
