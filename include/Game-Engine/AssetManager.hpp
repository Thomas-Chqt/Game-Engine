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

#include "Game-Engine/Mesh.hpp"

#include <Graphics/Device.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/CommandBuffer.hpp>

#include <atomic>
#include <cassert>
#include <cstddef>
#include <concepts>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <random>
#include <ranges>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace GE
{

template<typename T>
concept ManagableAsset = std::is_same_v<std::remove_cvref_t<T>, Mesh> || std::is_same_v<std::remove_cvref_t<T>, gfx::Texture>;

using AssetID = uint64_t;

class AssetManager
{
public:
    AssetManager() = delete;
    AssetManager(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;

    AssetManager(gfx::Device*);

    template<ManagableAsset T>
    AssetID registerAsset(const std::filesystem::path& path)
    {
        auto registredAssetsIt = m_registredAssets.find(path);
        if (registredAssetsIt == m_registredAssets.end())
        {
            uint64_t newAssetId = 0;
            static thread_local std::mt19937_64 rng(std::random_device{}());
            static thread_local std::uniform_int_distribution<uint64_t> dist;
            do newAssetId = dist(rng);
            while (newAssetId == 0 || m_assets.contains(newAssetId));
            registredAssetsIt = m_registredAssets.insert(std::make_pair(path, newAssetId)).first;
            auto [assetIt, inserted] = m_assets.try_emplace(registredAssetsIt->second, std::in_place_type<AssetHandle<T>>);
            assert(inserted);
            auto& handle = std::get<AssetHandle<T>>(assetIt->second);
            if constexpr (std::is_same_v<T, Mesh>)
            {
                handle.loader = [this, path=path](gfx::CommandBuffer& commandBuffer) -> std::shared_ptr<Mesh> {
                    return std::make_shared<Mesh>(loadMesh(path, commandBuffer));
                };
            }
            else if constexpr (std::is_same_v<T, gfx::Texture>)
            {
                handle.loader = [this, path=path](gfx::CommandBuffer& commandBuffer) -> std::shared_ptr<gfx::Texture> {
                    return loadTexture(path, commandBuffer);
                };
            }
        }
        return registredAssetsIt->second;
    }

    template<ManagableAsset T>
    const std::shared_future<const std::shared_ptr<T>&>& loadAsset(AssetID assetId)
    {
        auto it = m_assets.find(assetId);
        assert(it != m_assets.end());
        AssetHandle<T>& handle = std::get<AssetHandle<T>>(it->second);
        return loadAsset(handle);
    }

    const std::future<void> loadAssets(const std::ranges::sized_range auto& assetIds) requires std::convertible_to<std::ranges::range_value_t<decltype(assetIds)>, AssetID>
    {
        std::unique_ptr<gfx::CommandBufferPool> commandBufferPool = nullptr;
        std::shared_ptr<gfx::CommandBuffer> commandBuffer = nullptr;
        std::vector<std::future<void>> futures;
        futures.reserve(std::ranges::size(assetIds));

        for (AssetID assetId : assetIds)
        {
            auto it = m_assets.find(assetId);
            assert(it != m_assets.end());

            auto visitor = [&](auto& handle) {
                const decltype(handle.future)& future = loadAsset(handle);
                futures.push_back(std::async(std::launch::deferred, [future = future]() { future.get(); }));
            };
            std::visit(std::move(visitor), it->second);
        }
        return std::async(std::launch::deferred, [futures = std::move(futures)]() mutable {
            for (auto& f : futures)
                f.get();
        });
    }

    void unloadAsset(AssetID);

    template<ManagableAsset T>
    inline const std::shared_ptr<T>& getAsset(AssetID assetId) { return loadAsset<T>(assetId).get(); }

    ~AssetManager();

private:
    enum class LoadingStatus : std::uint8_t
    {
        unloaded,
        loading,
        loaded
    };

    template<ManagableAsset T>
    struct AssetHandle
    {
        std::function<std::shared_ptr<T>(gfx::CommandBuffer&)> loader;
        // std::function serializer ???
        std::atomic<LoadingStatus> status = LoadingStatus::unloaded;
        std::shared_future<const std::shared_ptr<T>&> future;
        std::shared_ptr<T> asset;
    };

    template<ManagableAsset T>
    const std::shared_future<const std::shared_ptr<T>&>& loadAsset(AssetHandle<T>& handle)
    {
        auto expected = LoadingStatus::unloaded;
        if (handle.status.compare_exchange_strong(expected, LoadingStatus::loading))
        {
            handle.future = std::async(std::launch::async, [device = m_device, handle = &handle]() -> const std::shared_ptr<T>& {
                std::unique_ptr<gfx::CommandBufferPool> commandBufferPool = device->newCommandBufferPool();
                assert(commandBufferPool);
                std::shared_ptr<gfx::CommandBuffer> commandBuffer = commandBufferPool->get();
                assert(commandBuffer);
                handle->asset = handle->loader(*commandBuffer);
                device->submitCommandBuffers(commandBuffer);
                handle->status.store(LoadingStatus::loaded);
                return handle->asset;
            });
        }
        return handle.future;
    }

    std::shared_ptr<gfx::Buffer> newDeviceLocalBuffer(gfx::CommandBuffer& commandBuffer, gfx::BufferUsage usage, const std::ranges::sized_range auto& data)
    {
        std::shared_ptr<gfx::Buffer> indexBuffer = m_device->newBuffer(gfx::Buffer::Descriptor{
            .size = sizeof(std::ranges::range_value_t<decltype(data)>) * data.size(),
            .usages = usage | gfx::BufferUsage::copyDestination,
            .storageMode = gfx::ResourceStorageMode::deviceLocal });
        assert(indexBuffer);

        std::shared_ptr<gfx::Buffer> stagingBuffer = m_device->newBuffer(gfx::Buffer::Descriptor{
            .size = indexBuffer->size(),
            .usages = gfx::BufferUsage::copySource,
            .storageMode = gfx::ResourceStorageMode::hostVisible });
        assert(stagingBuffer);

        std::ranges::copy(data, stagingBuffer->content<std::ranges::range_value_t<decltype(data)>>());

        commandBuffer.copyBufferToBuffer(stagingBuffer, indexBuffer, indexBuffer->size());

        return indexBuffer;
    }

    Mesh loadMesh(const std::filesystem::path&, gfx::CommandBuffer&);
    std::shared_ptr<gfx::Texture> loadTexture(const std::filesystem::path&, gfx::CommandBuffer&);
    std::shared_ptr<gfx::Texture> loadTexture(const std::byte* bytes, uint32_t width, uint32_t height, gfx::CommandBuffer&);

    gfx::Device* m_device = nullptr;
    std::map<std::filesystem::path, uint64_t> m_registredAssets;
    std::map<uint64_t, std::variant<AssetHandle<Mesh>, AssetHandle<gfx::Texture>>> m_assets;

public:
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;
};

} // namespace GE

#endif // ASSETMANAGER_HPP
