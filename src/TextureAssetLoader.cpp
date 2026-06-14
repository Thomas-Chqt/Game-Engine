/*
 * ---------------------------------------------------
 * TextureAssetLoader.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/TextureAssetLoader.hpp"

#include "Game-Engine/AssetContainer.hpp"
#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/TypeList.hpp"
#include "fastgltf/types.hpp"

#include <Graphics/Texture.hpp>

#include <glm/glm.hpp>
#include <stb_image/stb_image.h>

#include <array>
#include <concepts>
#include <cstddef>
#include <cassert>
#include <filesystem>
#include <span>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace GE
{

AssetLoader<gfx::Texture>::AssetLoader(gfx::Device* device, AssetManager* assetManager, const AssetLocation<gfx::Texture>& location)
    : AssetLoaderBase<gfx::Texture>(device, assetManager)
    , m_source(location)
{
}

AssetLoader<gfx::Texture>::AssetLoader(gfx::Device* device, AssetManager* assetManager, std::span<const std::byte> encodedBytes)
    : AssetLoaderBase<gfx::Texture>(device, assetManager)
    , m_source(encodedBytes)
{
}

AssetLoader<gfx::Texture>::AssetLoader(gfx::Device* device, AssetManager* assetManager, const std::byte* bytes, uint32_t width, uint32_t height)
    : AssetLoaderBase<gfx::Texture>(device, assetManager)
    , m_source(std::tuple(bytes, width, height))
{
}

AssetLoader<gfx::Texture>::AssetLoader(gfx::Device* device, AssetManager* assetManager, const glm::vec4& color)
    : AssetLoaderBase<gfx::Texture>(device, assetManager)
    , m_source(color)
{
    assert(color.r >= 0.0f && color.r <= 1.0f);
    assert(color.g >= 0.0f && color.g <= 1.0f);
    assert(color.b >= 0.0f && color.b <= 1.0f);
    assert(color.a >= 0.0f && color.a <= 1.0f);
}

std::shared_ptr<gfx::Texture> AssetLoader<gfx::Texture>::load(gfx::CommandBuffer& commandBuffer)
{
    return std::visit([&](const auto& source) {
        using SourceType = std::remove_cvref_t<decltype(source)>;
        if constexpr (std::same_as<SourceType, AssetLocation<gfx::Texture>>)
            return load(source, commandBuffer);
        if constexpr (std::same_as<SourceType, std::span<const std::byte>>)
            return load(source, commandBuffer);
        if constexpr (std::same_as<SourceType, std::tuple<const std::byte*, uint32_t, uint32_t>>) {
            auto& [bytes, width, height] = source;
            return load(bytes, width, height, commandBuffer);
        }
        if constexpr (std::same_as<SourceType, glm::vec4>)
            return load(source, commandBuffer);
        std::unreachable();
    }, m_source);
}

std::shared_ptr<gfx::Texture> AssetLoader<gfx::Texture>::load(const AssetLocation<gfx::Texture>& location, gfx::CommandBuffer& commandBuffer)
{
    assert(m_assetManager);
    std::shared_ptr<AssetContainer> container = m_assetManager->assetContainer(location.containerPath);

    if (auto* image = dynamic_cast<ImageAssetContainer*>(container.get())) {
        assert(location.index == 0);
        return load(image->path(), commandBuffer);
    }

    auto* gltfContainer = dynamic_cast<GltfAssetContainer*>(container.get());
    assert(gltfContainer);
    const fastgltf::Texture& texture = gltfContainer->asset().textures.at(location.index);
    assert(texture.imageIndex.has_value());
    const fastgltf::Image& image = gltfContainer->asset().images.at(*texture.imageIndex);

    auto imageSource = std::visit([&]<typename T>(const T& source) -> std::variant<std::filesystem::path, std::span<const std::byte>> {
        if constexpr (std::same_as<T, fastgltf::sources::BufferView>) {
            const fastgltf::BufferView& bufferView = gltfContainer->asset().bufferViews.at(source.bufferViewIndex);
            const fastgltf::Buffer& buffer = gltfContainer->asset().buffers.at(bufferView.bufferIndex);
            const std::span<const std::byte> bufferBytes = std::visit([&]<typename Y>(const Y& bufferSource) -> std::span<const std::byte> {
                if constexpr (!IsTypeInList<Y, TypeList<std::monostate, fastgltf::sources::BufferView, fastgltf::sources::URI, fastgltf::sources::CustomBuffer, fastgltf::sources::Fallback>>)
                    return std::span<const std::byte>(bufferSource.bytes.data(), bufferSource.bytes.size());
                else
                    throw std::runtime_error("unsuported buffer data");
            }, buffer.data);
            return bufferBytes.subspan(bufferView.byteOffset, bufferView.byteLength);
        }
        else if constexpr (std::same_as<T, fastgltf::sources::URI>) {
            assert(source.uri.isLocalPath());
            return gltfContainer->path().parent_path() / source.uri.fspath();
        }
        else if constexpr (!IsTypeInList<T, TypeList<std::monostate, fastgltf::sources::CustomBuffer, fastgltf::sources::Fallback>>)
            return std::span<const std::byte>(source.bytes.data(), source.bytes.size());
        else
            throw std::runtime_error("unsuported image data");
    }, image.data);

    return std::visit([&](const auto& source) -> std::shared_ptr<gfx::Texture> { return load(source, commandBuffer); }, imageSource);
}

std::shared_ptr<gfx::Texture> AssetLoader<gfx::Texture>::load(const std::filesystem::path& path, gfx::CommandBuffer& commandBuffer)
{
    ImageAssetContainer imageContainer{path};
    return load(imageContainer.encodedBytes(), commandBuffer);
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

std::shared_ptr<gfx::Texture> AssetLoader<gfx::Texture>::load(const glm::vec4& color, gfx::CommandBuffer& commandBuffer)
{
    assert(color.r >= 0.0f && color.r <= 1.0f);
    assert(color.g >= 0.0f && color.g <= 1.0f);
    assert(color.b >= 0.0f && color.b <= 1.0f);
    assert(color.a >= 0.0f && color.a <= 1.0f);

    const auto toByte = [](float value) {
        return static_cast<std::byte>(static_cast<uint8_t>(value * 255.0f));
    };
    const std::array<std::byte, 4> pixel = {
        toByte(color.r),
        toByte(color.g),
        toByte(color.b),
        toByte(color.a)
    };
    const std::array<std::byte, 16> pixels = {
        pixel[0], pixel[1], pixel[2], pixel[3],
        pixel[0], pixel[1], pixel[2], pixel[3],
        pixel[0], pixel[1], pixel[2], pixel[3],
        pixel[0], pixel[1], pixel[2], pixel[3]
    };
    return load(pixels.data(), 2, 2, commandBuffer);
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
