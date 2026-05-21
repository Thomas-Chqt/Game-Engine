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
#include "Game-Engine/TypeList.hpp"
#include "Game-Engine/ManagableAsset.hpp"
#include "Game-Engine/MeshAssetLoader.hpp" // IWYU pragma: keep
#include "Game-Engine/TextureAssetLoader.hpp" // IWYU pragma: keep

#include <Graphics/CommandBuffer.hpp>
#include <Graphics/Device.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/CommandBufferPool.hpp>

#include <yaml-cpp/yaml.h>

#include <optional>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <future>
#include <map>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include <string>
#include <concepts>

namespace GE
{

using AssetID = uint64_t;

template<typename T>
concept AssetIdRange = std::ranges::range<T> && std::convertible_to<std::ranges::range_value_t<T>, AssetID>;

constexpr AssetID BUILT_IN_CUBE_ID = 0;

/*
 * AssetManager is not thread safe, asset loading happen in a backgroud thread but calling registerAsset or loadAsset
 * from multiple thread without synchronization is **undefined behavior**
 */
class GE_API AssetManager
{
public:
    AssetManager() = delete;
    AssetManager(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;

    AssetManager(gfx::Device*);

    template<ManagableAsset T>
    AssetID registerAsset(const std::filesystem::path&, std::optional<AssetID> = std::nullopt);

    template<ManagableAsset T>
    std::shared_future<std::shared_ptr<T>> loadAsset(AssetID);

    [[nodiscard("destructing the future will wait for completion")]]
    std::future<void> loadAsset(AssetID);

    void loadAssetBackground(AssetID);

    [[nodiscard("destructing the future will wait for completion")]]
    std::future<void> loadAssets(const AssetIdRange auto&);

    void loadAssetsBackground(const AssetIdRange auto&);

    void unloadAsset(AssetID);
    void unloadAssets(const AssetIdRange auto&);

    bool isAssetLoaded(AssetID) const;
    bool areAssetsLoaded(const AssetIdRange auto&) const;

    template<ManagableAsset T>
    std::shared_ptr<T> getAsset(AssetID) const;
    std::vector<AssetID> assetIds() const;
    const std::string& assetName(AssetID) const;
    uint32_t assetLoadCount(AssetID) const;
    const std::optional<std::filesystem::path>& assetPath(AssetID) const;

    bool isValidAssetId(AssetID) const;
    template<ManagableAsset T>
    bool is(AssetID) const;

    ~AssetManager() = default;

private:
    template<ManagableAsset T>
    struct AssetHandle
    {
        using AssetType = T;

        std::optional<std::filesystem::path> path;
        std::string name;
        AssetLoader<T> loader;

        std::shared_future<std::shared_ptr<T>> future;
        uint32_t loadCount = 0;
    };

    using AssetHandleTypes = ManagableAssetTypes::wrapped<AssetHandle>;
    using VAssetHandle = AssetHandleTypes::into<std::variant>;

    template<ManagableAsset T>
    std::shared_future<std::shared_ptr<T>> loadAssetHandle(AssetHandle<T>&);

    gfx::Device* m_device = nullptr;

    // registered assets (with call to registerAsset) goes m_registeredAssets
    // asset that dont have file on disks or that are registered internally may not be in m_registeredAssets
    std::map<std::filesystem::path, AssetID> m_registeredAssets;
    std::map<AssetID, VAssetHandle> m_assetHandles;

public:
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;
};

template<ManagableAsset T>
AssetID AssetManager::registerAsset(const std::filesystem::path& path, std::optional<AssetID> assetId)
{
    assert(std::filesystem::is_regular_file(path));

    assert(assetId.has_value() == false || m_registeredAssets.contains(path) == false || m_registeredAssets.at(path) == assetId.value());

    auto [it, inserted] = m_registeredAssets.try_emplace(path);
    if (inserted) {
        if (assetId.has_value()) {
            assert(m_assetHandles.contains(assetId.value()) == false);
            it->second = assetId.value();
        } else {
            it->second = AssetID{0};
            while (m_assetHandles.contains(it->second)) // no ideal for performance but convenient
                it->second++;
        }
        auto [_, inserted] = m_assetHandles.insert(std::make_pair(it->second, AssetHandle<T>{
            .path = path,
            .name = path.stem().string(),
            .loader = AssetLoader<T>(m_device, std::move(path))
        }));
        assert(inserted);
    } else {
        // if path was already in m_registeredAssets, m_registeredAssets[path] should be in m_assetHandles
        assert(m_assetHandles.contains(it->second));
        assert(std::get<AssetHandle<T>>(m_assetHandles.at(it->second)).path.transform([&](auto& p){ return p == path; }).value_or(false));
    }

    return it->second;
}

template<ManagableAsset T>
std::shared_future<std::shared_ptr<T>> AssetManager::loadAsset(AssetID assetId)
{
    assert(m_assetHandles.contains(assetId));
    auto& handle = std::get<AssetHandle<T>>(m_assetHandles.at(assetId));
    return loadAssetHandle(handle);
}

std::future<void> AssetManager::loadAssets(const AssetIdRange auto& assetIds)
{
    assert(std::ranges::all_of(assetIds, [&](auto& id){ return m_assetHandles.contains(id); }));

    std::vector<std::future<void>> futures;
    if constexpr (std::ranges::sized_range<std::remove_cvref_t<decltype(assetIds)>>)
        futures.reserve(std::ranges::size(assetIds));

    for (const auto& assetId : assetIds)
        futures.push_back(loadAsset(assetId));

    return std::async(std::launch::async, [futures=std::move(futures)]() mutable {
        for (auto& future : futures)
            future.get();
    });
}

void AssetManager::loadAssetsBackground(const AssetIdRange auto& assetIds)
{
    assert(std::ranges::all_of(assetIds, [&](auto& id){ return isValidAssetId(id); }));
    std::ranges::for_each(assetIds, [this](const auto& assetId) { return loadAssetBackground(assetId); });
}

void AssetManager::unloadAssets(const AssetIdRange auto& assetIds)
{
    assert(std::ranges::all_of(assetIds, [&](auto& id){ return m_assetHandles.contains(id); }));
    std::ranges::for_each(assetIds, [this](const auto& assetId) { return unloadAsset(assetId); });
}

bool AssetManager::areAssetsLoaded(const AssetIdRange auto& assetIds) const
{
    assert(std::ranges::all_of(assetIds, [&](auto& id){ return m_assetHandles.contains(id); }));
    return std::ranges::all_of(assetIds, [this](const auto& assetId) { return isAssetLoaded(assetId); });
}

template<ManagableAsset T>
std::shared_ptr<T> AssetManager::getAsset(AssetID assetId) const
{
    assert(m_assetHandles.contains(assetId));
    auto& handle = std::get<AssetHandle<T>>(m_assetHandles.at(assetId));
    assert(handle.future.valid());
    return handle.future.get();
}

template<ManagableAsset T>
std::shared_future<std::shared_ptr<T>> AssetManager::loadAssetHandle(AssetHandle<T>& handle)
{
    handle.loadCount++;
    if (handle.future.valid() == false) // is not loading or already loaded
    {
        handle.future = std::async(std::launch::async, [device=m_device, &handle]() -> std::shared_ptr<T> {
            std::unique_ptr<gfx::CommandBufferPool> newCommandBufferPool = device->newCommandBufferPool();
            assert(newCommandBufferPool);
            std::shared_ptr<gfx::CommandBuffer> commandBuffer = newCommandBufferPool->get();
            assert(commandBuffer);
            std::shared_ptr<T> asset = handle.loader.load(*commandBuffer);
            device->submitCommandBuffers(commandBuffer);
            return asset;
        });
    }
    return handle.future;
}

template<ManagableAsset T>
bool AssetManager::is(AssetID assetId) const
{
    assert(m_assetHandles.contains(assetId));
    const VAssetHandle& vHandle = m_assetHandles.at(assetId);
    return std::holds_alternative<AssetHandle<T>>(vHandle);
}

} // namespace GE

#endif // ASSETMANAGER_HPP
