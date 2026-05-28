/*
 * ---------------------------------------------------
 * MeshAssetLoader.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef MESHASSETLOADER_HPP
#define MESHASSETLOADER_HPP

#include "Game-Engine/AssetLocation.hpp"
#include "Game-Engine/AssetLoader.hpp"
#include "Game-Engine/Mesh.hpp"

#include <Graphics/Device.hpp>
#include <Graphics/CommandBuffer.hpp>

#include <cstdint>
#include <variant>

namespace GE
{

class AssetManager;

enum class BuiltInMesh : uint8_t { cube };

template<>
class AssetLoader<Mesh> : public AssetLoaderBase<Mesh>
{
public:
    AssetLoader(const AssetLoader&) = delete;
    AssetLoader(AssetLoader&&) = default;

    AssetLoader(gfx::Device*, AssetManager*, const AssetLocation<Mesh>&);
    AssetLoader(gfx::Device*, AssetManager*, BuiltInMesh);

    std::shared_ptr<Mesh> load(gfx::CommandBuffer&) const;

    ~AssetLoader() = default;

private:
    std::variant<AssetLocation<Mesh>, BuiltInMesh> m_source;

    std::shared_ptr<Mesh> load(const AssetLocation<Mesh>&, gfx::CommandBuffer&) const;
    std::shared_ptr<Mesh> load(const BuiltInMesh&, gfx::CommandBuffer&) const;

public:
    AssetLoader& operator=(const AssetLoader&) = delete;
    AssetLoader& operator=(AssetLoader&&) = default;
};

}

#endif
