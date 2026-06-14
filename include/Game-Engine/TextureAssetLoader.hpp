/*
 * ---------------------------------------------------
 * TextureAssetLoader.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef TEXTUREASSETLOADER_HPP
#define TEXTUREASSETLOADER_HPP

#include "Game-Engine/AssetLocation.hpp"
#include "Game-Engine/AssetLoader.hpp"

#include <Graphics/Device.hpp>
#include <Graphics/CommandBuffer.hpp>
#include <Graphics/Texture.hpp>

#include <glm/glm.hpp>

#include <cstddef>
#include <filesystem>
#include <variant>

namespace GE
{

class AssetManager;

template<>
class AssetLoader<gfx::Texture> : public AssetLoaderBase<gfx::Texture>
{
public:
    AssetLoader(const AssetLoader&) = delete;
    AssetLoader(AssetLoader&&) = default;

    AssetLoader(gfx::Device*, AssetManager*, const AssetLocation<gfx::Texture>& location);
    AssetLoader(gfx::Device*, AssetManager*, std::span<const std::byte> encodedBytes);
    AssetLoader(gfx::Device*, AssetManager*, const std::byte* bytes, uint32_t width, uint32_t height);
    AssetLoader(gfx::Device*, AssetManager*, const glm::vec4& color);

    std::shared_ptr<gfx::Texture> load(gfx::CommandBuffer&);

    ~AssetLoader() = default;

private:
    std::shared_ptr<gfx::Texture> load(const AssetLocation<gfx::Texture>&, gfx::CommandBuffer&);
    std::shared_ptr<gfx::Texture> load(const std::filesystem::path&, gfx::CommandBuffer&);
    std::shared_ptr<gfx::Texture> load(const std::span<const std::byte>& encodedBytes, gfx::CommandBuffer&);
    std::shared_ptr<gfx::Texture> load(const std::byte* bytes, uint32_t width, uint32_t height, gfx::CommandBuffer&);
    std::shared_ptr<gfx::Texture> load(const glm::vec4& color, gfx::CommandBuffer&);

    using ImageDataSource = std::variant<
        AssetLocation<gfx::Texture>,
        std::span<const std::byte>,
        std::tuple<const std::byte*, uint32_t, uint32_t>,
        glm::vec4
    >;

    ImageDataSource m_source;

public:
    AssetLoader& operator=(const AssetLoader&) = delete;
    AssetLoader& operator=(AssetLoader&&) = default;
};

} // namespace GE

#endif // TEXTUREASSETLOADER_HPP
