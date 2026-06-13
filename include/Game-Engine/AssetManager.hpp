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

#include "Game-Engine/AssetContainer.hpp"
#include "Game-Engine/AssetLocation.hpp"
#include "Game-Engine/AssetLoader.hpp"
#include "Game-Engine/Export.hpp"
#include "Game-Engine/MeshAssetLoader.hpp" // IWYU pragma: keep
#include "Game-Engine/TextureAssetLoader.hpp" // IWYU pragma: keep
#include "Game-Engine/ThreadPool.hpp"

#include <Graphics/CommandBuffer.hpp>
#include <Graphics/CommandBufferPool.hpp>
#include <Graphics/Device.hpp>
#include <Graphics/Texture.hpp>

#include <algorithm>
#include <array>
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
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace GE
{

using AssetID = uint64_t;

template<typename T>
concept AssetIdRange = std::ranges::range<T> && std::convertible_to<std::ranges::range_value_t<T>, AssetID>;

constexpr AssetID BUILT_IN_CUBE_ID = 0;

class TextureTable;

class GE_API AssetManager
{
public:
    AssetManager() = delete;
    AssetManager(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;

    AssetManager(gfx::Device*, ThreadPool*);

    void importGltf(const std::filesystem::path&);

    template<ManagableAsset T>
    AssetID registerAsset(std::string_view name, const std::filesystem::path& containerPath, std::size_t idx = 0);

    AssetID registerAsset(std::optional<AssetID>, std::string_view name, const VAssetLocation&, const AssetIdRange auto& dependentAssets);

    template<ManagableAsset T>
    AssetID registerAsset(
        std::optional<AssetID>,
        std::string_view name,
        const std::optional<AssetLocation<T>>&,
        const AssetIdRange auto& dependentAssets,
        AssetLoader<T>&&);

    template<ManagableAsset T>
    std::shared_future<std::shared_ptr<T>> loadAsset(AssetID);

    std::shared_future<void> loadAsset(AssetID);

    [[nodiscard("destructing the future will wait for completion")]]
    std::future<void> loadAssets(const AssetIdRange auto&);

    void loadAssetsDetached(const AssetIdRange auto&);

    void unloadAsset(AssetID);
    void unloadAssets(const AssetIdRange auto&);

    template<ManagableAsset T>
    std::shared_ptr<T> getAsset(AssetID) const;

    auto assetIds() const;
    std::set<std::filesystem::path> assetContainerPaths() const;
    AssetID assetId(const VAssetLocation&) const;
    std::string_view assetName(AssetID assetId) const;
    std::span<const AssetID> assetDependencies(AssetID) const;
    std::optional<VAssetLocation> assetLocation(AssetID) const;
    uint32_t assetLoadCount(AssetID) const;

    template<ManagableAsset T>
    bool assetTypeIs(AssetID) const;

    bool isRegistered(const VAssetLocation&) const;
    bool isValidAssetId(AssetID) const;
    bool isAssetLoaded(AssetID) const;
    bool isAssetContainerLoaded(const std::filesystem::path&) const;
    bool areAssetsLoaded(const AssetIdRange auto&) const;

    std::shared_ptr<AssetContainer> assetContainer(const std::filesystem::path&);
    void attachTextureTable(const std::shared_ptr<TextureTable>&);

    ~AssetManager() = default;

private:
    enum class LoadStatus : uint8_t
    {
        unloaded,
        loading,
        loaded,
        error
    };

    template<ManagableAsset T>
    struct AssetHandle
    {
        using AssetType = T;

        std::string name;
        std::optional<AssetLocation<T>> location;
        std::vector<AssetID> dependentAssets;
        AssetLoader<T> loader;

        uint32_t loadCount = 0;
        std::atomic<LoadStatus> loadStatus = LoadStatus::unloaded;

        std::shared_future<std::shared_ptr<T>> future;
        std::shared_future<void> voidFuture;
    };

    using AssetHandleTypes = ManagableAssetTypes::wrapped<AssetHandle>;
    using VAssetHandle = AssetHandleTypes::into<std::variant>;

    void setTexture(AssetID, const std::shared_ptr<gfx::Texture>&);
    void removeTexture(AssetID);

    gfx::Device* m_device = nullptr;
    ThreadPool* m_threadPool = nullptr;

    std::map<VAssetLocation, AssetID> m_registeredAssets;
    std::map<std::filesystem::path, std::weak_ptr<AssetContainer>> m_assetContainers;
    std::map<AssetID, VAssetHandle> m_assetHandles;
    std::weak_ptr<TextureTable> m_textureTable;

    mutable std::mutex m_assetContainersMutex;

public:
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;
};

template<ManagableAsset T>
AssetID AssetManager::registerAsset(std::string_view name, const std::filesystem::path& containerPath, std::size_t idx)
{
    return registerAsset<T>(std::nullopt, name, AssetLocation<T>{containerPath, idx}, std::array<AssetID, 0>{}, AssetLoader<T>(m_device, this, AssetLocation<T>{containerPath, idx}));
}

AssetID AssetManager::registerAsset(std::optional<AssetID> assetId, std::string_view name, const VAssetLocation& vLocation, const AssetIdRange auto& dependentAssets)
{
    return std::visit([&]<typename T>(const AssetLocation<T>& location) -> AssetID {
        return registerAsset<T>(assetId, name, location, dependentAssets, AssetLoader<T>{m_device, this, location});
    }, vLocation);
}

template<ManagableAsset T>
AssetID AssetManager::registerAsset(std::optional<AssetID> assetId, std::string_view name, const std::optional<AssetLocation<T>>& location, const AssetIdRange auto& dependentAssets, AssetLoader<T>&& loader)
{
    if (location.has_value() && isRegistered(*location)) {
        if (assetId.has_value())
            assert(this->assetId(*location) == *assetId);
        return this->assetId(*location);
    }

    AssetID newAssetId = 0;
    if (assetId.has_value())
        newAssetId = *assetId;
    else {
        // TODO : find a better way to get a valid asset id
        while (m_assetHandles.contains(newAssetId))
            newAssetId++;
    }

    m_assetHandles.try_emplace(newAssetId, std::in_place_type<AssetHandle<T>>,
        std::string(name),
        std::move(location),
        dependentAssets | std::ranges::to<std::vector>(),
        std::move(loader)
    );

    if (location.has_value())
        m_registeredAssets.try_emplace(*location, newAssetId);

    return newAssetId;
}

template<ManagableAsset T>
std::shared_future<std::shared_ptr<T>> AssetManager::loadAsset(AssetID assetId)
{
    assert(isValidAssetId(assetId));
    assert(assetTypeIs<T>(assetId));

    AssetHandle<T>& handle = std::get<AssetHandle<T>>(m_assetHandles.at(assetId));

    std::future<void> dependentAssetsFuture;
    if (handle.dependentAssets.empty() == false)
        dependentAssetsFuture = loadAssets(handle.dependentAssets);

    handle.loadCount++;
    auto expected = LoadStatus::unloaded;
    if (handle.loadStatus.compare_exchange_strong(expected, LoadStatus::loading))
    {
        assert(handle.loadCount == 1);
        assert(handle.future.valid() == false);
        assert(handle.voidFuture.valid() == false);

        std::promise<void> promise;
        handle.voidFuture = promise.get_future().share();
        handle.future = m_threadPool->submit([this, assetId, &handle, promise=std::move(promise), dependentAssetsFuture=std::move(dependentAssetsFuture)] mutable -> std::shared_ptr<T> {
            try {
                std::unique_ptr<gfx::CommandBufferPool> commandBufferPool = m_device->newCommandBufferPool();
                assert(commandBufferPool);
                std::shared_ptr<gfx::CommandBuffer> commandBuffer = commandBufferPool->get();
                assert(commandBuffer);

                std::shared_ptr<T> asset = handle.loader.load(*commandBuffer);
                m_device->submitCommandBuffers(commandBuffer);
                if constexpr (std::same_as<T, gfx::Texture>)
                    setTexture(assetId, asset);
                else
                    (void)assetId;
                promise.set_value();
                if (dependentAssetsFuture.valid())
                    dependentAssetsFuture.get();
                handle.loadStatus.store(LoadStatus::loaded);
                return asset;
            } catch (...) {
                handle.loadStatus.store(LoadStatus::error);
                promise.set_exception(std::current_exception());
                throw;
            }
        }).share();
    }

    return handle.future;
}

std::future<void> AssetManager::loadAssets(const AssetIdRange auto& assetIds)
{
    assert(std::ranges::all_of(assetIds, [this](AssetID assetId) { return isValidAssetId(assetId); }));

    auto futures = assetIds | std::views::transform([&](AssetID id) -> std::shared_future<void> { return loadAsset(id); });

    return m_threadPool->submit([futures=futures|std::ranges::to<std::vector>()] mutable {
        for (auto& future : futures)
            future.get();
    });
}

void AssetManager::loadAssetsDetached(const AssetIdRange auto& assetIds)
{
    assert(std::ranges::all_of(assetIds, [this](AssetID assetId) { return isValidAssetId(assetId); }));

    for(AssetID id : assetIds)
        loadAsset(id);
}

void AssetManager::unloadAssets(const AssetIdRange auto& assetIds)
{
    assert(std::ranges::all_of(assetIds, [this](AssetID assetId) { return isValidAssetId(assetId); }));

    for(AssetID id : assetIds)
        unloadAsset(id);
}

template<ManagableAsset T>
std::shared_ptr<T> AssetManager::getAsset(AssetID assetId) const
{
    assert(isValidAssetId(assetId));
    assert(assetTypeIs<T>(assetId));
    const AssetHandle<T>& handle = std::get<AssetHandle<T>>(m_assetHandles.at(assetId));
    assert(handle.future.valid());
    return handle.future.get();
}

inline auto AssetManager::assetIds() const
{
    return m_assetHandles | std::views::keys;
}

template<ManagableAsset T>
bool AssetManager::assetTypeIs(AssetID assetId) const
{
    assert(isValidAssetId(assetId));
    return std::holds_alternative<AssetHandle<T>>(m_assetHandles.at(assetId));
}

bool AssetManager::areAssetsLoaded(const AssetIdRange auto& assetIds) const
{
    return std::ranges::all_of(assetIds
        | std::views::transform([&](AssetID id) -> const VAssetHandle& { return m_assetHandles.at(id); })
        | std::views::transform([&](const VAssetHandle& vHandle){
            return std::visit([]<ManagableAsset T>(const AssetHandle<T>& handle) {
                return handle.future.valid() && handle.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
            }, vHandle);
        })
    );
}

} // namespace GE

#endif // ASSETMANAGER_HPP
