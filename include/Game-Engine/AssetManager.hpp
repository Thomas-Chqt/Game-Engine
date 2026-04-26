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

#include "Game-Engine/Export.hpp"
#include "Game-Engine/Mesh.hpp"
#include "Game-Engine/TypeList.hpp"

#include <Graphics/Device.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/CommandBuffer.hpp>

#include <yaml-cpp/yaml.h>

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
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include <algorithm>

namespace GE
{

using ManagableAssetTypes = TypeList<Mesh, gfx::Texture>;

template<typename T>
concept ManagableAsset = IsTypeInList<std::remove_cvref_t<T>, ManagableAssetTypes>;

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

template<typename T>
struct AssetPathYamlTraits;

template<>
struct AssetPathYamlTraits<AssetPath<Mesh>>
{
    static constexpr std::string_view name = "Mesh";
};

template<>
struct AssetPathYamlTraits<AssetPath<gfx::Texture>>
{
    static constexpr std::string_view name = "Texture";
};

template<typename AssetT>
using AssetPathType = AssetPath<AssetT>;

using AssetPathTypes = ManagableAssetTypes::wrapped<AssetPathType>;
using VAssetPath = AssetPathTypes::into<std::variant>;

template<typename T>
concept VAssetPathRange = std::ranges::range<T> && std::is_same_v<std::ranges::range_value_t<T>, VAssetPath>;

class GE_API AssetManager
{
public:
    AssetManager() = delete;
    AssetManager(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;

    AssetManager(gfx::Device*);

    void registerAsset(const VAssetPath&);

    template<ManagableAsset T>
    inline const std::shared_future<const std::shared_ptr<T>&>& loadAsset(const VAssetPath& vAssetPath) { return loadAssetHandle<T>(m_handles.at(vAssetPath)); }

    std::future<void> loadAssets(VAssetPathRange auto&& vAssetPaths) {
        std::vector<std::future<void>> futures;
        if constexpr (std::ranges::sized_range<std::remove_cvref_t<decltype(vAssetPaths)>>)
            futures.reserve(std::ranges::size(vAssetPaths));

        for (const VAssetPath& vAssetPath : vAssetPaths)
        {
            std::visit([&](const auto& assetPath) {
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

    inline const std::shared_future<const std::shared_ptr<Mesh>&>& loadBuiltInCube() { return loadAssetHandle<Mesh>(m_builtInCubeHandle); }

    inline bool isAssetLoaded(const VAssetPath& vAssetPath) const { return isAssetHandleLoaded(m_handles.at(vAssetPath)); }

    bool areAssetsLoaded(VAssetPathRange auto&& vAssetPaths) const {
        return std::ranges::all_of(vAssetPaths, [this](const VAssetPath& vAssetPath) {
            return isAssetLoaded(vAssetPath);
        });
    }

    inline bool isBuiltInCubeLoaded() const { return isAssetHandleLoaded(m_builtInCubeHandle); }

    inline void unloadAsset(const VAssetPath& vAssetPath) { unloadAssetHandle(m_handles.at(vAssetPath)); }

    void unloadAssets(VAssetPathRange auto&& vAssetPaths) {
        for (const VAssetPath& vAssetPath : vAssetPaths)
            unloadAsset(vAssetPath);
    }

    inline void unloadBuiltInCube() { unloadAssetHandle(m_builtInCubeHandle); }

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

    template<typename AssetT>
    using AssetHandleType = AssetHandle<AssetT>;

    using AssetHandleTypes = ManagableAssetTypes::wrapped<AssetHandleType>;
    using VAssetHandle = AssetHandleTypes::into<std::variant>;

    template<ManagableAsset T>
    const std::shared_future<const std::shared_ptr<T>&>& loadAssetHandle(VAssetHandle& vHandle) {
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

    bool isAssetHandleLoaded(const VAssetHandle& vHandle) const {
        return std::visit([](const auto& handle) {
            return handle.status.load() == AssetHandleLoadingStatus::loaded;
        },
        vHandle);
    }

    void unloadAssetHandle(VAssetHandle&);

    static std::shared_ptr<gfx::Buffer> newDeviceLocalBuffer(gfx::Device& device, gfx::CommandBuffer& commandBuffer, gfx::BufferUsage usage, const std::ranges::sized_range auto& data) {
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

namespace YAML
{

template<>
struct convert<GE::VAssetPath>
{
    static Node encode(const GE::VAssetPath& rhs)
    {
        Node node;
        std::visit([&](auto& assetPath)
        {
            using AssetPathT = std::remove_cvref_t<decltype(assetPath)>;
            node["type"] = std::string(GE::AssetPathYamlTraits<AssetPathT>::name);
            node["path"] = std::string(assetPath.path);
        },
        rhs);
        return node;
    }

    static bool decode(const Node& node, GE::VAssetPath& rhs)
    {
        if (!node.IsMap() || !node["type"] || !node["path"])
            return false;

        const std::string type = node["type"].as<std::string>();
        const std::filesystem::path path = node["path"].as<std::string>();
        bool isDecoded = false;

        GE::forEachType<GE::AssetPathTypes>([&]<typename AssetPathT>() {
            if (isDecoded || type != GE::AssetPathYamlTraits<AssetPathT>::name)
                return;

            rhs = AssetPathT(path);
            isDecoded = true;
        });

        return isDecoded;
    }
};

}
#endif // ASSETMANAGER_HPP
