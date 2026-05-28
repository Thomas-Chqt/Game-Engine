#ifndef ASSETCONTAINER_HPP
#define ASSETCONTAINER_HPP

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>

#include <cstddef>
#include <filesystem>
#include <span>
#include <vector>

namespace GE
{

class AssetContainer
{
public:
    AssetContainer() = default;
    AssetContainer(const AssetContainer&) = delete;
    AssetContainer(AssetContainer&&) = delete;

    virtual const std::filesystem::path& path() const = 0;

    virtual ~AssetContainer() = default;

public:
    AssetContainer& operator=(const AssetContainer&) = delete;
    AssetContainer& operator=(AssetContainer&&) = delete;
};

class ImageAssetContainer final : public AssetContainer
{
public:
    ImageAssetContainer() = delete;
    ImageAssetContainer(const ImageAssetContainer&) = delete;
    ImageAssetContainer(ImageAssetContainer&&) = delete;

    explicit ImageAssetContainer(std::filesystem::path);

    const std::filesystem::path& path() const override;
    std::span<const std::byte> encodedBytes() const;

    ~ImageAssetContainer() override = default;

private:
    std::filesystem::path m_path;
    std::vector<std::byte> m_encodedBytes;

public:
    ImageAssetContainer& operator=(const ImageAssetContainer&) = delete;
    ImageAssetContainer& operator=(ImageAssetContainer&&) = delete;
};

class GltfAssetContainer final : public AssetContainer
{
public:
    GltfAssetContainer() = delete;
    GltfAssetContainer(const GltfAssetContainer&) = delete;
    GltfAssetContainer(GltfAssetContainer&&) = delete;

    explicit GltfAssetContainer(std::filesystem::path);

    const std::filesystem::path& path() const override;
    const fastgltf::Asset& asset() const;

    ~GltfAssetContainer() override = default;

private:
    std::filesystem::path m_path;
    fastgltf::GltfDataBuffer m_data;
    fastgltf::Asset m_asset;

public:
    GltfAssetContainer& operator=(const GltfAssetContainer&) = delete;
    GltfAssetContainer& operator=(GltfAssetContainer&&) = delete;
};

} // namespace GE

#endif // ASSETCONTAINER_HPP
