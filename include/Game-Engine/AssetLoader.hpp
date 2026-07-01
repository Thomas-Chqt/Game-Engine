/*
 * ---------------------------------------------------
 * AssetLoader.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef ASSETLOADER_HPP
#define ASSETLOADER_HPP

#include "Game-Engine/ManagableAsset.hpp"

#include <Graphics/Device.hpp>
#include <Graphics/CommandBuffer.hpp>

#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>

namespace GE
{

class AssetManager;

template<ManagableAsset T>
class AssetLoaderBase
{
protected:
    AssetLoaderBase(gfx::Device* device, AssetManager* assetManager);

    gfx::Device* m_device = nullptr;
    AssetManager* m_assetManager = nullptr;

    std::shared_ptr<gfx::Buffer> newDeviceLocalBuffer(gfx::Device& device, gfx::CommandBuffer& commandBuffer, gfx::BufferUsage usage, const std::ranges::sized_range auto& data) const;
};

template<ManagableAsset T>
AssetLoaderBase<T>::AssetLoaderBase(gfx::Device* device, AssetManager* assetManager)
    : m_device(device)
    , m_assetManager(assetManager)
{
}

template<ManagableAsset T>
std::shared_ptr<gfx::Buffer> AssetLoaderBase<T>::newDeviceLocalBuffer(gfx::Device& device, gfx::CommandBuffer& commandBuffer, gfx::BufferUsage usage, const std::ranges::sized_range auto& data) const
{
    std::shared_ptr<gfx::Buffer> deviceBuffer = device.newBuffer(gfx::Buffer::Descriptor{
        .size = sizeof(std::ranges::range_value_t<std::remove_cvref_t<decltype(data)>>) * data.size(),
        .usages = usage | gfx::BufferUsage::copyDestination,
        .storageMode = gfx::ResourceStorageMode::deviceLocal
    });
    assert(deviceBuffer);

    std::shared_ptr<gfx::Buffer> stagingBuffer = device.newBuffer(gfx::Buffer::Descriptor{
        .size = deviceBuffer->size(),
        .usages = gfx::BufferUsage::copySource,
        .storageMode = gfx::ResourceStorageMode::hostVisible
    });
    assert(stagingBuffer);

    std::ranges::copy(data, stagingBuffer->content<std::ranges::range_value_t<std::remove_cvref_t<decltype(data)>>>());
    commandBuffer.copyBufferToBuffer(stagingBuffer, deviceBuffer, deviceBuffer->size());
    return deviceBuffer;
}

template<ManagableAsset T>
class AssetLoader : public AssetLoaderBase<T>
{
public:
    AssetLoader() = delete;
    AssetLoader(const AssetLoader&) = delete;
    AssetLoader(AssetLoader&&) = default;

    AssetLoader(gfx::Device*, AssetManager*, std::function<std::shared_ptr<T>(gfx::CommandBuffer&)> loaderFn);

    std::shared_ptr<T> load(gfx::CommandBuffer&) const;

    ~AssetLoader();

private:
    std::function<std::shared_ptr<T>(gfx::CommandBuffer&)> m_loader;

public:
    AssetLoader& operator=(const AssetLoader&) = delete;
    AssetLoader& operator=(AssetLoader&&) = default;
};

template<ManagableAsset T>
AssetLoader<T>::AssetLoader(gfx::Device* device, AssetManager* assetManager, std::function<std::shared_ptr<T>(gfx::CommandBuffer&)> loaderFn)
    : AssetLoaderBase<T>(device, assetManager)
    , m_loader(std::move(loaderFn))
{
}

template<ManagableAsset T>
std::shared_ptr<T> AssetLoader<T>::load(gfx::CommandBuffer& commandBuffer) const
{
    return m_loader(commandBuffer);
}

}

#endif
