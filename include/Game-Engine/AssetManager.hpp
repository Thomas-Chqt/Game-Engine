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
#include <ranges>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace GE
{

template<typename T>
concept ManagableAsset = std::is_same_v<std::remove_cvref_t<T>, Mesh> || std::is_same_v<std::remove_cvref_t<T>, gfx::Texture>;

using AssetID = std::size_t;

class AssetManager
{
public:
    AssetManager() = delete;
    AssetManager(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;

    AssetManager(gfx::Device*);

    template<ManagableAsset T>
    AssetID registerAsset(const auto& id) requires requires {{ std::hash<std::remove_cvref_t<decltype(id)>>{}(id) } -> std::convertible_to<std::size_t>; }
    {
        auto [it, success] = m_assets.try_emplace(std::hash<std::remove_cvref_t<decltype(id)>>{}(id), std::in_place_type<AssetHandle<T>>);
        assert(success); // lets see if collision really happen
        if (success)
        {
            if constexpr (std::is_same_v<T, Mesh>)
            {
                static_assert(std::is_same_v<std::remove_cvref_t<decltype(id)>, std::filesystem::path>, "for meshes, id must be a path to the mesh");

                AssetHandle<Mesh>& handle = std::get<AssetHandle<Mesh>>(it->second);
                handle.loader = [this, path = id](gfx::CommandBuffer& commandBuffer) -> std::unique_ptr<Mesh> {
                    return std::make_unique<Mesh>(loadMesh(path, commandBuffer));
                };
            }
            else if constexpr (std::is_same_v<T, gfx::Texture>)
            {
                static_assert(std::is_same_v<std::remove_cvref_t<decltype(id)>, std::filesystem::path>, "for textures, id must be a path to the texture");
                static_assert(false, "not implemented");
            }
        }
        return it->first;
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
        std::function<std::unique_ptr<T>(gfx::CommandBuffer&)> loader;
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

    Mesh loadMesh(const std::filesystem::path&, gfx::CommandBuffer&);

    gfx::Device* m_device = nullptr;
    std::map<AssetID, std::variant<AssetHandle<Mesh>, AssetHandle<gfx::Texture>>> m_assets;

public:
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;
};

} // namespace GE

#endif // ASSETMANAGER_HPP
