/*
 * ---------------------------------------------------
 * AssetManager.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/13 11:49:48
 * ---------------------------------------------------
 */

#ifndef ASSETMANAGER_HPP
#define ASSETMANAGER_HPP

#include "Game-Engine/AssetLoader.hpp"
#include "Game-Engine/Export.hpp"
#include "Game-Engine/ManagableAsset.hpp"
#include "Game-Engine/MeshAssetLoader.hpp" // IWYU pragma: keep
#include "Game-Engine/TextureAssetLoader.hpp" // IWYU pragma: keep
#include "Game-Engine/TypeList.hpp"

#include <Graphics/CommandBuffer.hpp>
#include <Graphics/CommandBufferPool.hpp>
#include <Graphics/Device.hpp>
#include <Graphics/Texture.hpp>

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace GE
{

using AssetID = uint64_t;

template<typename T>
concept AssetIdRange = std::ranges::range<T> && std::convertible_to<std::ranges::range_value_t<T>, AssetID>;

constexpr AssetID BUILT_IN_CUBE_ID = 0;

class GE_API AssetManager
{
public:
    AssetManager() = delete;
    AssetManager(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;

    AssetManager(gfx::Device*);

    template<ManagableAsset T>
    AssetID registerAsset(std::string_view name, const std::filesystem::path&, std::optional<AssetID> = std::nullopt);

    template<ManagableAsset T>
    AssetID registerAsset(std::string_view name, AssetLoader<T>&&, std::optional<AssetID> = std::nullopt);

    template<ManagableAsset T>
    std::shared_future<std::shared_ptr<T>> loadAsset(AssetID);

    [[nodiscard("destructing the future will wait for completion")]]
    std::future<void> loadAsset(AssetID);

    [[nodiscard("destructing the future will wait for completion")]]
    std::future<void> loadAssets(const AssetIdRange auto&);

    void loadAssetDetached(AssetID);
    void loadAssetsDetached(const AssetIdRange auto&);

    void unloadAsset(AssetID);
    void unloadAssets(const AssetIdRange auto&);

    template<ManagableAsset T>
    std::shared_ptr<T> getAsset(AssetID) const;

    std::set<AssetID> assetIds() const;
    AssetID assetId(const std::filesystem::path&) const;
    std::string assetName(AssetID) const;
    std::optional<std::filesystem::path> assetPath(AssetID) const;
    uint32_t assetLoadCount(AssetID) const;

    template<ManagableAsset T>
    bool assetTypeIs(AssetID) const;

    bool isRegistered(const std::filesystem::path&) const;
    bool isValidAssetId(AssetID) const;
    bool isAssetLoaded(AssetID) const;
    bool areAssetsLoaded(const AssetIdRange auto&) const;

    ~AssetManager() = default;

private:
    template<ManagableAsset T>
    class AssetHandle
    {
    public:
        using AssetType = T;

        AssetHandle(std::string_view, std::optional<std::filesystem::path>, AssetLoader<T>&&);

        std::string name() const;
        std::optional<std::filesystem::path> path() const;
        uint32_t loadCount() const;
        std::shared_future<std::shared_ptr<T>> future() const;

        std::shared_future<std::shared_ptr<T>> load(gfx::Device*, gfx::CommandBufferPool*);
        void unload();
        bool isLoaded() const;

    private:
        std::string m_name;
        std::optional<std::filesystem::path> m_path;
        AssetLoader<T> m_loader;

        mutable std::mutex m_mutex;
        std::shared_future<std::shared_ptr<T>> m_future;
        uint32_t m_loadCount = 0;
    };

    using AssetHandleTypes = ManagableAssetTypes::wrapped<AssetHandle>;
    using VAssetHandle = AssetHandleTypes::into<std::variant>;

    template<ManagableAsset T>
    AssetID registerAssetHandle(std::optional<AssetID>, std::string_view name, std::optional<std::filesystem::path>, AssetLoader<T>&&);

    decltype(auto) withCommandBufferPool(const std::invocable<gfx::CommandBufferPool&> auto& fn);

    gfx::Device* m_device = nullptr;

    mutable std::mutex m_registryMutex;
    std::map<std::filesystem::path, AssetID> m_registeredAssets;
    std::map<AssetID, VAssetHandle> m_assetHandles;

public:
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;
};


template<ManagableAsset T>
AssetID AssetManager::registerAsset(std::string_view name, const std::filesystem::path& path, std::optional<AssetID> assetId)
{
    std::lock_guard lock(m_registryMutex);

    #ifndef NDEBUG
    assert(std::filesystem::is_regular_file(path));
    if (const auto registeredAssetIt = m_registeredAssets.find(path); registeredAssetIt != m_registeredAssets.end()) {
        const AssetID registeredAssetId = registeredAssetIt->second;
        if (assetId.has_value())
            assert(registeredAssetId == assetId.value());
        const VAssetHandle& vHandle = m_assetHandles.at(registeredAssetId);
        assert(std::holds_alternative<AssetHandle<T>>(vHandle));
        const AssetHandle<T>& handle = std::get<AssetHandle<T>>(vHandle);
        assert(handle.name() == name);
        assert(handle.path() == path);
    }
    else if (assetId.has_value()) {
        assert(m_assetHandles.contains(assetId.value()) == false);
    }
    #endif

    auto [it, inserted] = m_registeredAssets.try_emplace(path);
    if (inserted) {
        it->second = registerAssetHandle<T>(assetId, name, path, AssetLoader<T>(m_device, path));
    }
    return it->second;
}

template<ManagableAsset T>
AssetID AssetManager::registerAsset(std::string_view name, AssetLoader<T>&& loader, std::optional<AssetID> assetId)
{
    std::lock_guard lock(m_registryMutex);

    #ifndef NDEBUG
    if (assetId.has_value() && m_assetHandles.contains(assetId.value())) {
        const VAssetHandle& vHandle = m_assetHandles.at(*assetId);
        assert(std::holds_alternative<AssetHandle<T>>(vHandle));
        const AssetHandle<T>& handle = std::get<AssetHandle<T>>(vHandle);
        assert(handle.name() == name);
        assert(handle.path().has_value() == false);
    }
    #endif

    return registerAssetHandle<T>(assetId, name, std::nullopt, std::move(loader));
}

template<ManagableAsset T>
std::shared_future<std::shared_ptr<T>> AssetManager::loadAsset(AssetID assetId)
{
    AssetHandle<T>& handle = [&]() -> AssetHandle<T>& {
        std::scoped_lock lock(m_registryMutex);
        assert(m_assetHandles.contains(assetId));
        return std::get<AssetHandle<T>>(m_assetHandles.at(assetId));
    }();
    return handle.load(m_device, nullptr);
}

std::future<void> AssetManager::loadAssets(const AssetIdRange auto& assetIds)
{
    std::vector<VAssetHandle*> vHandles;
    if constexpr (std::ranges::sized_range<std::remove_cvref_t<decltype(assetIds)>>)
        vHandles.reserve(assetIds.size());
    {
        std::scoped_lock lock(m_registryMutex);
        assert(std::ranges::all_of(assetIds, [this](AssetID assetId) {
            return m_assetHandles.contains(assetId);
        }));
        for (AssetID id : assetIds)
            vHandles.push_back(&m_assetHandles.at(id));
    };
    std::vector<std::future<void>> futures;
    futures.reserve(vHandles.size());
    withCommandBufferPool([&](gfx::CommandBufferPool& commandBufferPool) {
        for (VAssetHandle* vHandle : vHandles) {
            futures.push_back(std::visit([&]<ManagableAsset T>(AssetHandle<T>& handle) -> std::future<void> {
                std::shared_future<std::shared_ptr<T>> future = handle.load(m_device, &commandBufferPool);
                return std::async(std::launch::deferred, [future]() { future.get(); });
            }, *vHandle));
        }
    });
    return std::async(std::launch::async, [futures=std::move(futures)]() mutable {
        for (auto& future : futures)
            future.get();
    });
}

void AssetManager::loadAssetsDetached(const AssetIdRange auto& assetIds)
{
    std::vector<VAssetHandle*> vHandles;
    if constexpr (std::ranges::sized_range<std::remove_cvref_t<decltype(assetIds)>>)
        vHandles.reserve(assetIds.size());
    {
        std::scoped_lock lock(m_registryMutex);
        assert(std::ranges::all_of(assetIds, [this](AssetID assetId) {
            return m_assetHandles.contains(assetId);
        }));
        for (AssetID id : assetIds)
            vHandles.push_back(&m_assetHandles.at(id));
    };
    withCommandBufferPool([&](gfx::CommandBufferPool& commandBufferPool) {
        for (VAssetHandle* vHandle : vHandles) {
            std::visit([&]<ManagableAsset T>(AssetHandle<T>& handle) {
                handle.load(m_device, &commandBufferPool);
            }, *vHandle);
        }
    });
}

void AssetManager::unloadAssets(const AssetIdRange auto& assetIds)
{
    std::vector<VAssetHandle*> vHandles;
    if constexpr (std::ranges::sized_range<std::remove_cvref_t<decltype(assetIds)>>)
        vHandles.reserve(assetIds.size());
    {
        std::scoped_lock lock(m_registryMutex);
        assert(std::ranges::all_of(assetIds, [this](AssetID assetId) {
            return m_assetHandles.contains(assetId);
        }));
        #ifndef NDEBUG
        for (AssetID id : assetIds) {
            const VAssetHandle& vHandle = m_assetHandles.at(id);
            assert(std::visit([]<ManagableAsset T>(const AssetHandle<T>& handle) {
                return handle.isLoaded();
            }, vHandle));
        }
        #endif
        for (AssetID id : assetIds)
            vHandles.push_back(&m_assetHandles.at(id));
    };
    for (VAssetHandle* vHandle : vHandles) {
        std::visit([&]<ManagableAsset T>(AssetHandle<T>& handle) {
            handle.unload();
        }, *vHandle);
    }
}

template<ManagableAsset T>
std::shared_ptr<T> AssetManager::getAsset(AssetID assetId) const
{
    const AssetHandle<T>& handle = [&]() -> const AssetHandle<T>& {
        std::scoped_lock lock(m_registryMutex);
        assert(m_assetHandles.contains(assetId));
        return std::get<AssetHandle<T>>(m_assetHandles.at(assetId));
    }();
    return handle.future().get();
}

template<ManagableAsset T>
bool AssetManager::assetTypeIs(AssetID assetId) const
{
    std::lock_guard lock(m_registryMutex);
    assert(m_assetHandles.contains(assetId));
    return std::holds_alternative<AssetHandle<T>>(m_assetHandles.at(assetId));
}

bool AssetManager::areAssetsLoaded(const AssetIdRange auto& assetIds) const
{
    std::vector<const VAssetHandle*> vHandles;
    if constexpr (std::ranges::sized_range<std::remove_cvref_t<decltype(assetIds)>>)
        vHandles.reserve(assetIds.size());
    {
        std::scoped_lock lock(m_registryMutex);
        assert(std::ranges::all_of(assetIds, [this](AssetID assetId) {
            return m_assetHandles.contains(assetId);
        }));
        for (AssetID id : assetIds)
            vHandles.push_back(&m_assetHandles.at(id));
    };

    return std::ranges::all_of(vHandles, [](const auto* vHandle) {
        return std::visit([&]<ManagableAsset T>(const AssetHandle<T>& handle) {
            return handle.isLoaded();
        }, *vHandle);
    });
}

/* ------------ PRIVATE ------------ */

template<ManagableAsset T>
AssetManager::AssetHandle<T>::AssetHandle(std::string_view assetName, std::optional<std::filesystem::path> assetPath, AssetLoader<T>&& assetLoader)
    : m_name(assetName)
    , m_path(std::move(assetPath))
    , m_loader(std::move(assetLoader))
{
}

template<ManagableAsset T>
std::string AssetManager::AssetHandle<T>::name() const
{
    std::lock_guard lock(m_mutex);
    return m_name;
}

template<ManagableAsset T>
std::optional<std::filesystem::path> AssetManager::AssetHandle<T>::path() const
{
    std::lock_guard lock(m_mutex);
    return m_path;
}

template<ManagableAsset T>
uint32_t AssetManager::AssetHandle<T>::loadCount() const
{
    std::lock_guard lock(m_mutex);
    return m_loadCount;
}

template<ManagableAsset T>
std::shared_future<std::shared_ptr<T>> AssetManager::AssetHandle<T>::future() const
{
    std::lock_guard lock(m_mutex);
    return m_future;
}

template<ManagableAsset T>
std::shared_future<std::shared_ptr<T>> AssetManager::AssetHandle<T>::load(gfx::Device* device, gfx::CommandBufferPool* commandBufferPool)
{
    assert(device);
    std::lock_guard lock(m_mutex);
    m_loadCount++;
    if (m_future.valid() == false) {
        std::unique_ptr<gfx::CommandBufferPool> localCommandBufferPool;
        if (commandBufferPool == nullptr) {
            localCommandBufferPool = device->newCommandBufferPool();
            assert(localCommandBufferPool);
            commandBufferPool = localCommandBufferPool.get();
        }

        std::shared_ptr<gfx::CommandBuffer> commandBuffer = commandBufferPool->get();
        assert(commandBuffer);

        m_future = std::async(std::launch::async, [this, device, commandBuffer]() -> std::shared_ptr<T> {
            std::shared_ptr<T> asset = m_loader.load(*commandBuffer);
            device->submitCommandBuffers(commandBuffer);
            return asset;
        });
    }
    return m_future;
}

template<ManagableAsset T>
void AssetManager::AssetHandle<T>::unload()
{
    std::lock_guard lock(m_mutex);
    assert(m_loadCount > 0);
    assert(m_future.valid());
    m_loadCount--;
    if (m_loadCount == 0) {
        m_future.wait();
        m_future = std::shared_future<std::shared_ptr<T>>();
    }
}

template<ManagableAsset T>
bool AssetManager::AssetHandle<T>::isLoaded() const
{
    std::lock_guard lock(m_mutex);
    #ifndef NDEBUG
    if (m_loadCount > 0)
        assert(m_future.valid());
    else
        assert(m_future.valid() == false);
    #endif
    return m_future.valid() && m_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

template<ManagableAsset T>
AssetID AssetManager::registerAssetHandle(std::optional<AssetID> assetId, std::string_view name, std::optional<std::filesystem::path> path, AssetLoader<T>&& loader)
{
    #ifndef NDEBUG
    if (assetId.has_value() && m_assetHandles.contains(*assetId)) {
        const VAssetHandle& vHandle = m_assetHandles.at(*assetId);
        assert(std::holds_alternative<AssetHandle<T>>(vHandle));
        const AssetHandle<T>& handle = std::get<AssetHandle<T>>(vHandle);
        assert(handle.name() == name);
        assert(handle.path() == path);
    }
    #endif

    AssetID newAssetId = 0;
    if (assetId.has_value())
        newAssetId = *assetId;
    else {
        while (m_assetHandles.contains(newAssetId))
            newAssetId++;
    }

    auto [it, inserted] = m_assetHandles.try_emplace(newAssetId, std::in_place_type<AssetHandle<T>>, name, std::move(path), std::move(loader));
    assert(inserted || assetId.has_value());

    return it->first;
}

decltype(auto) AssetManager::withCommandBufferPool(const std::invocable<gfx::CommandBufferPool&> auto& fn)
{
    std::unique_ptr<gfx::CommandBufferPool> commandBufferPool = m_device->newCommandBufferPool();
    assert(commandBufferPool);
    return fn(*commandBufferPool);
}

} // namespace GE

#endif // ASSETMANAGER_HPP
