#include "Game-Engine/AssetContainer.hpp"

#include <fstream>
#include <ios>
#include <stdexcept>
#include <utility>

namespace gltf = fastgltf;

namespace GE
{

namespace
{

std::vector<std::byte> readBinaryFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("failed to open file: " + path.string());

    const std::size_t size = std::filesystem::file_size(path);
    std::vector<std::byte> bytes(size);
    file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    if (!file)
        throw std::runtime_error("failed to read file: " + path.string());
    return bytes;
}

} // namespace

ImageAssetContainer::ImageAssetContainer(std::filesystem::path path)
    : m_path(std::move(path))
    , m_encodedBytes(readBinaryFile(m_path))
{
}

const std::filesystem::path& ImageAssetContainer::path() const
{
    return m_path;
}

std::span<const std::byte> ImageAssetContainer::encodedBytes() const
{
    return m_encodedBytes;
}

GltfAssetContainer::GltfAssetContainer(std::filesystem::path path)
    : m_path(std::move(path))
{
    gltf::Expected<gltf::GltfDataBuffer> gltfDataBuffer = gltf::GltfDataBuffer::FromPath(m_path);
    if (gltfDataBuffer.error() != gltf::Error::None)
        throw std::runtime_error(m_path.string() + ": failed to load glTF: data error");
    m_data = std::move(gltfDataBuffer.get());

    gltf::Parser parser;
    gltf::Expected<gltf::Asset> asset = parser.loadGltf(
        m_data,
        m_path.parent_path(),
        gltf::Options::LoadExternalBuffers | gltf::Options::GenerateMeshIndices,
        gltf::Category::Asset
        | gltf::Category::Buffers
        | gltf::Category::BufferViews
        | gltf::Category::Accessors
        | gltf::Category::Images
        | gltf::Category::Textures
        | gltf::Category::Materials
        | gltf::Category::Meshes
        | gltf::Category::Nodes
        | gltf::Category::Scenes
    );
    if (asset.error() != gltf::Error::None)
        throw std::runtime_error(m_path.string() + ": failed to load glTF: asset error");
    m_asset = std::move(asset.get());
}

const std::filesystem::path& GltfAssetContainer::path() const
{
    return m_path;
}

const gltf::Asset& GltfAssetContainer::asset() const
{
    return m_asset;
}

} // namespace GE
