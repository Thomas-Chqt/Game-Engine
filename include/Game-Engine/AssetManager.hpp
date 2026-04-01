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
#include <cstdint>
#include <filesystem>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace GE
{

template<typename T>
concept ManagableAsset = std::is_same_v<std::remove_cvref_t<T>, Mesh> || std::is_same_v<std::remove_cvref_t<T>, gfx::Texture>;

template<ManagableAsset T>
struct AssetPath
{
    using AssetType = T;
    std::filesystem::path path;
    AssetPath() = default;
    AssetPath(std::filesystem::path path) : path(std::move(path)) {}
    inline operator std::filesystem::path& () { return path; }
    inline operator const std::filesystem::path& () const { return path; }
    friend bool operator==(const AssetPath&, const AssetPath&) = default;
    friend bool operator<(const AssetPath& lhs, const AssetPath& rhs) { return lhs.path < rhs.path; }
};

using VAssetPath = std::variant<AssetPath<Mesh>, AssetPath<gfx::Texture>>;

template<typename T>
concept VAssetPathSizedRange = std::ranges::sized_range<T> && std::is_same_v<std::ranges::range_value_t<T>, VAssetPath>;

class AssetManager
{
public:
    AssetManager() = delete;
    AssetManager(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;

    AssetManager(gfx::Device*);

    void registerAsset(const VAssetPath&);

    template<ManagableAsset T>
    inline const std::shared_future<const std::shared_ptr<T>&>& loadAsset(const VAssetPath& vAssetPath) { return loadAssetHandle<T>(m_handles.at(vAssetPath)); }

    inline const std::shared_future<const std::shared_ptr<Mesh>&>& loadBuiltInCube() { return loadAssetHandle<Mesh>(m_builtInCubeHandle); }

    const std::future<void> loadAssets(const VAssetPathSizedRange auto& vAssetPaths)
    {
        std::vector<std::future<void>> futures;
        futures.reserve(std::ranges::size(vAssetPaths));

        for (VAssetPath& vAssetPath : vAssetPaths)
        {
            std::visit([&](auto& assetPath) {
                using AssetType = typename std::remove_cvref_t<decltype(assetPath)>::AssetType;
                const auto& future = loadAsset<AssetType>(vAssetPath);
                futures.push_back(std::async(std::launch::deferred, [future = future]() { future.get(); }));
            },
            vAssetPath);
        }
        return std::async(std::launch::deferred, [futures = std::move(futures)]() mutable {
            for (auto& f : futures)
                f.get();
        });
    }

    inline void unloadAsset(const VAssetPath& vAssetPath) { unloadAssetHandle(m_handles.at(vAssetPath)); }

    template<ManagableAsset T>
    inline const std::shared_ptr<T>& getAsset(VAssetPath& vAssetPath) { return loadAsset<T>(vAssetPath).get(); }

    ~AssetManager();

private:
    enum class AssetHandleLoadingStatus : std::uint8_t
    {
        unloaded,
        loading,
        loaded
    };

    template<ManagableAsset T>
    struct AssetHandle
    {
        using AssetType = T;

        std::function<std::shared_ptr<T>(gfx::CommandBuffer&)> loader;
        std::atomic<AssetHandleLoadingStatus> status = AssetHandleLoadingStatus::unloaded;
        std::shared_future<const std::shared_ptr<T>&> future;
        std::shared_ptr<T> asset;
    };

    using VAssetHandle = std::variant<AssetHandle<Mesh>, AssetHandle<gfx::Texture>>;

    template<ManagableAsset T>
    const std::shared_future<const std::shared_ptr<T>&>& loadAssetHandle(VAssetHandle& vHandle)
    {
        auto& handle = std::get<AssetHandle<T>>(vHandle);
        auto expected = AssetHandleLoadingStatus::unloaded;
        if (handle.status.compare_exchange_strong(expected, AssetHandleLoadingStatus::loading))
        {
            handle.future = std::async(std::launch::async, [device = m_device, handle = &handle]() -> const std::shared_ptr<T>& {
                std::unique_ptr<gfx::CommandBufferPool> commandBufferPool = device->newCommandBufferPool();
                assert(commandBufferPool);
                std::shared_ptr<gfx::CommandBuffer> commandBuffer = commandBufferPool->get();
                assert(commandBuffer);
                handle->asset = handle->loader(*commandBuffer);
                device->submitCommandBuffers(commandBuffer);
                handle->status.store(AssetHandleLoadingStatus::loaded);
                return handle->asset;
            });
        }
        return handle.future;
    }

    void unloadAssetHandle(VAssetHandle&);

    static std::shared_ptr<gfx::Buffer> newDeviceLocalBuffer(gfx::Device& device, gfx::CommandBuffer& commandBuffer, gfx::BufferUsage usage, const std::ranges::sized_range auto& data)
    {
        std::shared_ptr<gfx::Buffer> indexBuffer = device.newBuffer(gfx::Buffer::Descriptor{
            .size = sizeof(std::ranges::range_value_t<decltype(data)>) * data.size(),
            .usages = usage | gfx::BufferUsage::copyDestination,
            .storageMode = gfx::ResourceStorageMode::deviceLocal });
        assert(indexBuffer);

        std::shared_ptr<gfx::Buffer> stagingBuffer = device.newBuffer(gfx::Buffer::Descriptor{
            .size = indexBuffer->size(),
            .usages = gfx::BufferUsage::copySource,
            .storageMode = gfx::ResourceStorageMode::hostVisible });
        assert(stagingBuffer);

        std::ranges::copy(data, stagingBuffer->content<std::ranges::range_value_t<decltype(data)>>());

        commandBuffer.copyBufferToBuffer(stagingBuffer, indexBuffer, indexBuffer->size());

        return indexBuffer;
    }

    static Mesh loadMesh(gfx::Device&, const std::filesystem::path&, gfx::CommandBuffer&);
    static std::shared_ptr<gfx::Texture> loadTexture(gfx::Device&, const std::filesystem::path&, gfx::CommandBuffer&);
    static std::shared_ptr<gfx::Texture> loadTexture(gfx::Device&, const std::byte* bytes, uint32_t width, uint32_t height, gfx::CommandBuffer&);
    static Mesh loadBuiltInCube(gfx::Device&, gfx::CommandBuffer&);

    gfx::Device* m_device = nullptr;
    std::map<VAssetPath, VAssetHandle> m_handles;
    VAssetHandle m_builtInCubeHandle;

public:
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;
};

} // namespace GE

#endif // ASSETMANAGER_HPP
