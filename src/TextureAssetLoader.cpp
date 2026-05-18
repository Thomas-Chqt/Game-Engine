/*
 * ---------------------------------------------------
 * TextureAssetLoader.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/TextureAssetLoader.hpp"

#include <Graphics/Texture.hpp>

#include <stb_image/stb_image.h>

#include <concepts>
#include <cstddef>
#include <cassert>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace GE
{

AssetLoader<gfx::Texture>::AssetLoader(gfx::Device* device, std::filesystem::path path)
    : m_device(device), m_source(path)
{
}

AssetLoader<gfx::Texture>::AssetLoader(gfx::Device* device, std::span<const std::byte> encodedBytes)
    : m_device(device), m_source(encodedBytes)
{
}

AssetLoader<gfx::Texture>::AssetLoader(gfx::Device* device, const std::byte* bytes, uint32_t width, uint32_t height)
    : m_device(device), m_source(std::tuple(bytes, width, height))
{
}

std::shared_ptr<gfx::Texture> AssetLoader<gfx::Texture>::load(gfx::CommandBuffer& commandBuffer)
{
    return std::visit([&](const auto& source) {
        using SourceType = std::remove_cvref_t<decltype(source)>;
        if constexpr (std::same_as<SourceType, std::filesystem::path>)
            return load(source, commandBuffer);
        if constexpr (std::same_as<SourceType, std::span<const std::byte>>)
            return load(source, commandBuffer);
        if constexpr (std::same_as<SourceType, std::tuple<const std::byte*, uint32_t, uint32_t>>) {
            auto& [bytes, width, height] = source;
            return load(bytes, width, height, commandBuffer);
        }
        std::unreachable();
    }, m_source);
}

std::shared_ptr<gfx::Texture> AssetLoader<gfx::Texture>::load(const std::filesystem::path& path, gfx::CommandBuffer& commandBuffer)
{
    using UniqueStbiUc = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;
    assert(std::filesystem::is_regular_file(path));

    int width = 0;
    int height = 0;
    UniqueStbiUc bytes = UniqueStbiUc(stbi_load(path.string().c_str(), &width, &height, nullptr, STBI_rgb_alpha), stbi_image_free);
    if (!bytes)
        throw std::runtime_error("failed to load texture: " + path.string());
    return load(reinterpret_cast<const std::byte*>(bytes.get()), static_cast<uint32_t>(width), static_cast<uint32_t>(height), commandBuffer); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
}

std::shared_ptr<gfx::Texture> AssetLoader<gfx::Texture>::load(const std::span<const std::byte>& encodedBytes, gfx::CommandBuffer& commandBuffer)
{
    using UniqueStbiUc = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;

    int width = 0;
    int height = 0;
    UniqueStbiUc bytes = UniqueStbiUc(
        stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(encodedBytes.data()), static_cast<int>(encodedBytes.size()), &width, &height, nullptr, STBI_rgb_alpha), // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        stbi_image_free);
    if (!bytes)
        throw std::runtime_error("failed to load embedded texture");
    return load(reinterpret_cast<const std::byte*>(bytes.get()), static_cast<uint32_t>(width), static_cast<uint32_t>(height), commandBuffer); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
}

std::shared_ptr<gfx::Texture> AssetLoader<gfx::Texture>::load(const std::byte* bytes, uint32_t width, uint32_t height, gfx::CommandBuffer& commandBuffer)
{
    assert(bytes);
    assert(width > 0);
    assert(height > 0);

    std::shared_ptr<gfx::Texture> texture = m_device->newTexture(gfx::Texture::Descriptor{
        .type = gfx::TextureType::texture2d,
        .width = width,
        .height = height,
        .pixelFormat = gfx::PixelFormat::RGBA8Unorm,
        .usages = gfx::TextureUsage::copyDestination | gfx::TextureUsage::shaderRead,
        .storageMode = gfx::ResourceStorageMode::deviceLocal
    });
    assert(texture);

    std::shared_ptr<gfx::Buffer> stagingBuffer = m_device->newBuffer(gfx::Buffer::Descriptor{
        .size = static_cast<size_t>(width) * static_cast<size_t>(height) * pixelFormatSize(gfx::PixelFormat::RGBA8Unorm),
        .usages = gfx::BufferUsage::copySource,
        .storageMode = gfx::ResourceStorageMode::hostVisible
    });
    assert(stagingBuffer);

    std::memcpy(stagingBuffer->content<uint32_t>(), bytes, stagingBuffer->size());

    commandBuffer.beginBlitPass();
    commandBuffer.copyBufferToTexture(stagingBuffer, texture);
    commandBuffer.endBlitPass();

    return texture;
}

}
