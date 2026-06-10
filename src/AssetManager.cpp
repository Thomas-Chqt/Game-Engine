/*
 * ---------------------------------------------------
 * AssetManager.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/13 12:00:28
 * ---------------------------------------------------
 */

#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/AssetContainer.hpp"
#include "Game-Engine/AssetLocation.hpp"
#include "Game-Engine/ManagableAsset.hpp"
#include "Game-Engine/Mesh.hpp"
#include "Game-Engine/MeshAssetLoader.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <future>
#include <mutex>
#include <ranges>
#include <stdexcept>
#include <string>
#include <variant>

namespace GE
{

namespace
{

bool isGltfPath(const std::filesystem::path& path)
{
    std::string extension = path.extension().string();
    std::ranges::transform(extension, extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".gltf" || extension == ".glb";
}

bool isImagePath(const std::filesystem::path& path)
{
    std::string extension = path.extension().string();
    std::ranges::transform(extension, extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp" || extension == ".tga";
}

} // namespace

AssetManager::AssetManager(gfx::Device* device)
    : m_device(device)
{
    assert(m_device != nullptr);

    const AssetID builtInCubeId = registerAsset<Mesh>(
        "build_in_cube",
        AssetLoader<Mesh>(m_device, this, BuiltInMesh::cube),
        BUILT_IN_CUBE_ID
    );
    assert(builtInCubeId == BUILT_IN_CUBE_ID);
}

AssetID AssetManager::registerAsset(std::string_view name, const VAssetLocation& location, std::optional<AssetID> assetId)
{
    std::lock_guard lock(m_registryMutex);

    #ifndef NDEBUG
    const std::filesystem::path& containerPath = std::visit([](const auto& typedLocation) -> const std::filesystem::path& {
        return typedLocation.containerPath;
    }, location);
    assert(std::filesystem::is_regular_file(containerPath));
    if (const auto registeredAssetIt = m_registeredAssets.find(location); registeredAssetIt != m_registeredAssets.end()) {
        const AssetID registeredAssetId = registeredAssetIt->second;
        if (assetId.has_value())
            assert(registeredAssetId == assetId.value());
        assert(m_assetHandles.contains(registeredAssetId));
        std::visit([&](const auto& typedLocation) {
            using T = typename std::remove_cvref_t<decltype(typedLocation)>::AssetType;
            const VAssetHandle& vHandle = m_assetHandles.at(registeredAssetId);
            assert(std::holds_alternative<AssetHandle<T>>(vHandle));
            const AssetHandle<T>& handle = std::get<AssetHandle<T>>(vHandle);
            assert(handle.name() == name);
            assert(handle.location() == location);
        }, location);
    }
    else if (assetId.has_value()) {
        assert(m_assetHandles.contains(assetId.value()) == false);
    }
    #endif

    auto [it, inserted] = m_registeredAssets.try_emplace(location);
    if (inserted) {
        it->second = std::visit([&]<typename T>(const AssetLocation<T>& typedLocation) {
            return registerAssetHandle<T>(assetId, name, location, AssetLoader<T>(m_device, this, typedLocation));
        }, location);
    }
    return it->second;
}

std::shared_future<void> AssetManager::loadAsset(AssetID assetId)
{
    VAssetHandle& vHandle = [&]() -> VAssetHandle& {
        std::scoped_lock lock(m_registryMutex);
        assert(m_assetHandles.contains(assetId));
        return m_assetHandles.at(assetId);
    }();

    return std::visit([&]<ManagableAsset T>(AssetHandle<T>& handle) -> std::shared_future<void> {
        return handle.loadVoid(m_device, nullptr);
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

std::set<std::filesystem::path> AssetManager::assetContainerPaths() const
{
    std::lock_guard lock(m_registryMutex);
    std::set<std::filesystem::path> containerPaths;
    for (const auto& [path, _] : m_assetContainers)
        containerPaths.insert(path);
    for (const auto& [location, _] : m_registeredAssets) {
        std::visit([&](const auto& typedLocation) {
            containerPaths.insert(typedLocation.containerPath);
        }, location);
    }
    return containerPaths;
}

AssetID AssetManager::assetId(const VAssetLocation& location) const
{
    std::scoped_lock lock(m_registryMutex);
    assert(m_registeredAssets.contains(location));
    return m_registeredAssets.at(location);
}

std::string AssetManager::assetName(AssetID assetId) const
{
    const VAssetHandle& vHandle = [&]() -> const VAssetHandle& {
        std::scoped_lock lock(m_registryMutex);
        assert(m_assetHandles.contains(assetId));
        return m_assetHandles.at(assetId);
    }();

    return std::visit([]<ManagableAsset T>(const AssetHandle<T>& handle) -> std::string {
        return handle.name();
    }, vHandle);
}

std::optional<VAssetLocation> AssetManager::assetLocation(AssetID assetId) const
{
    const VAssetHandle& vHandle = [&]() -> const VAssetHandle& {
        std::scoped_lock lock(m_registryMutex);
        assert(m_assetHandles.contains(assetId));
        return m_assetHandles.at(assetId);
    }();

    return std::visit([]<ManagableAsset T>(const AssetHandle<T>& handle) -> std::optional<VAssetLocation> {
        return handle.location();
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

bool AssetManager::isRegistered(const VAssetLocation& location) const
{
    std::lock_guard lock(m_registryMutex);
    return m_registeredAssets.contains(location);
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

bool AssetManager::isAssetContainerLoaded(const std::filesystem::path& path) const
{
    std::lock_guard lock(m_registryMutex);
    const auto it = m_assetContainers.find(path);
    return it != m_assetContainers.end() && it->second.expired() == false;
}

std::shared_ptr<AssetContainer> AssetManager::assetContainer(const std::filesystem::path& path)
{
    std::lock_guard lock(m_registryMutex);

    if (auto it = m_assetContainers.find(path); it != m_assetContainers.end()) {
        if (std::shared_ptr<AssetContainer> container = it->second.lock())
            return container;
    }

    std::shared_ptr<AssetContainer> container;
    if (isImagePath(path))
        container = std::make_shared<ImageAssetContainer>(path);
    else if (isGltfPath(path))
        container = std::make_shared<GltfAssetContainer>(path);
    else
        throw std::runtime_error("unsupported asset container: " + path.string());

    m_assetContainers[path] = container;
    return container;
}

} // namespace GE
