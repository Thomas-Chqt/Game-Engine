/*
 * ---------------------------------------------------
 * MeshAssetLoader.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef MESHASSETLOADER_HPP
#define MESHASSETLOADER_HPP

#include "Game-Engine/AssetLoader.hpp"
#include "Game-Engine/Mesh.hpp"

#include <Graphics/Device.hpp>
#include <Graphics/CommandBuffer.hpp>

#include <cstdint>
#include <filesystem>
#include <variant>

namespace GE
{

enum class BuiltInMesh : uint8_t { cube };

template<>
class AssetLoader<Mesh> : public AssetLoaderBase<Mesh>
{
public:
    AssetLoader(const AssetLoader&) = delete;
    AssetLoader(AssetLoader&&) = default;

    AssetLoader(gfx::Device* device, std::filesystem::path);
    AssetLoader(gfx::Device* device, BuiltInMesh);

    std::shared_ptr<Mesh> load(gfx::CommandBuffer&) const;

    ~AssetLoader() = default;

private:
    gfx::Device* m_device;
    std::variant<std::filesystem::path, BuiltInMesh> m_source;

    std::shared_ptr<Mesh> load(const std::filesystem::path&, gfx::CommandBuffer&) const;
    std::shared_ptr<Mesh> load(const BuiltInMesh&, gfx::CommandBuffer&) const;

public:
    AssetLoader& operator=(const AssetLoader&) = delete;
    AssetLoader& operator=(AssetLoader&&) = default;
};

}

#endif
