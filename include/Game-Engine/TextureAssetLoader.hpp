/*
 * ---------------------------------------------------
 * TextureAssetLoader.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef TEXTUREASSETLOADER_HPP
#define TEXTUREASSETLOADER_HPP

#include "Game-Engine/AssetLoader.hpp"

#include <Graphics/Device.hpp>
#include <Graphics/CommandBuffer.hpp>
#include <Graphics/Texture.hpp>

#include <filesystem>
#include <variant>

namespace GE
{

template<>
class AssetLoader<gfx::Texture> : public AssetLoaderBase<gfx::Texture>
{
public:
    AssetLoader(const AssetLoader&) = delete;
    AssetLoader(AssetLoader&&) = default;

    AssetLoader(gfx::Device* device, std::filesystem::path);
    AssetLoader(gfx::Device* device, std::span<const std::byte> encodedBytes);
    AssetLoader(gfx::Device* device, const std::byte* bytes, uint32_t width, uint32_t height);

    std::shared_ptr<gfx::Texture> load(gfx::CommandBuffer&);

    ~AssetLoader() = default;

private:
    std::shared_ptr<gfx::Texture> load(const std::filesystem::path&, gfx::CommandBuffer&);
    std::shared_ptr<gfx::Texture> load(const std::span<const std::byte>& encodedBytes, gfx::CommandBuffer&);
    std::shared_ptr<gfx::Texture> load(const std::byte* bytes, uint32_t width, uint32_t height, gfx::CommandBuffer&);

    using ImageDataSource = std::variant<
        std::filesystem::path,
        std::span<const std::byte>,
        std::tuple<const std::byte*, uint32_t, uint32_t>
    >;

    gfx::Device* m_device;
    ImageDataSource m_source;

public:
    AssetLoader& operator=(const AssetLoader&) = delete;
    AssetLoader& operator=(AssetLoader&&) = default;
};

} // namespace GE

#endif // TEXTUREASSETLOADER_HPP
