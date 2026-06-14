/*
 * ---------------------------------------------------
 * AssetManager.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/13 12:00:28
 * ---------------------------------------------------
 */

#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/AssetContainer.hpp"
#include "Game-Engine/AssetLocation.hpp"
#include "Game-Engine/ManagableAsset.hpp"
#include "Game-Engine/Mesh.hpp"
#include "Game-Engine/MeshAssetLoader.hpp"
#include "Graphics/Texture.hpp"
#include "TextureTable.hpp"
#include "fastgltf/types.hpp"

#include <glm/glm.hpp>
#include <tracy/Tracy.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <format>
#include <future>
#include <optional>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>

namespace GE
{

namespace
{

bool isGltfPath(const std::filesystem::path& path)
{
    std::string extension = path.extension().string();
    std::ranges::transform(extension, extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".gltf" || extension == ".glb";
}

bool isImagePath(const std::filesystem::path& path)
{
    std::string extension = path.extension().string();
    std::ranges::transform(extension, extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp" || extension == ".tga";
}

} // namespace

AssetManager::AssetManager(gfx::Device* device, ThreadPool* threadPool)
    : m_device(device)
    , m_threadPool(threadPool)
{
    assert(m_device != nullptr);
    assert(m_threadPool != nullptr);

    const glm::vec4 whiteColor(1.0f);
    const AssetID builtInWhiteTextureId = registerAsset<gfx::Texture>(
        BUILT_IN_WHITE_TEXTURE_ID,
        "built_in_white_texture",
        std::nullopt,
        std::array<AssetID, 0>(),
        AssetLoader<gfx::Texture>(m_device, this, whiteColor)
    );
    assert(builtInWhiteTextureId == BUILT_IN_WHITE_TEXTURE_ID);

    const AssetID builtInCubeId = registerAsset<Mesh>(
        BUILT_IN_CUBE_ID,
        "build_in_cube",
        std::nullopt,
        std::array<AssetID, 1>{ BUILT_IN_WHITE_TEXTURE_ID },
        AssetLoader<Mesh>(m_device, this, BuiltInMesh::cube)
    );
    assert(builtInCubeId == BUILT_IN_CUBE_ID);
}

void AssetManager::importGltf(const std::filesystem::path& path)
{
    ZoneScopedN("AssetManager::importGltf");

    std::shared_ptr<AssetContainer> container = assetContainer(path);
    auto* gltfContainer = dynamic_cast<GltfAssetContainer*>(container.get());
    assert(gltfContainer);

    const fastgltf::Asset& asset = gltfContainer->asset();
    if (asset.scenes.empty())
        throw std::runtime_error(std::format("{}: failed to load glTF: no scene", path.string()));
    const fastgltf::Scene& scene = asset.defaultScene ? asset.scenes[*asset.defaultScene] : asset.scenes.front();

    std::set<AssetID> dependentAssets;

    auto registerDependentTexture = [&](const fastgltf::TextureInfo& textureInfo) {
        fastgltf::Texture texure = asset.textures.at(textureInfo.textureIndex);
        assert(texure.imageIndex.has_value());
        std::string name = texure.name.empty() == false
            ? std::string(texure.name)
            : asset.images[*texure.imageIndex].name.empty() == false
            ? std::string(asset.images[*texure.imageIndex].name)
            : std::string(std::format("{}#texture{}", path.stem().string(), textureInfo.textureIndex));
        dependentAssets.insert(registerAsset<gfx::Texture>(name, path, textureInfo.textureIndex));
    };

    auto processNode = [&](this auto&& self, const fastgltf::Node& node) -> void {
        if (fastgltf::Optional<std::size_t> meshIndex = node.meshIndex) {
            for (const fastgltf::Primitive& primitive : asset.meshes[*meshIndex].primitives) {
                if (primitive.materialIndex.has_value() == false) {
                    dependentAssets.insert(BUILT_IN_WHITE_TEXTURE_ID);
                    continue;
                }
                const fastgltf::Material& material = asset.materials[*primitive.materialIndex];
                if(material.pbrData.baseColorTexture.has_value())
                    registerDependentTexture(material.pbrData.baseColorTexture.value());
                else
                    dependentAssets.insert(BUILT_IN_WHITE_TEXTURE_ID);
                if(material.pbrData.metallicRoughnessTexture.has_value())
                    registerDependentTexture(material.pbrData.metallicRoughnessTexture.value());
                if(material.normalTexture.has_value())
                    registerDependentTexture(material.normalTexture.value());
                if(material.emissiveTexture.has_value())
                    registerDependentTexture(material.emissiveTexture.value());
            }
        }
        for (const fastgltf::Node& childNode : node.children | std::views::transform([&](std::size_t i) -> fastgltf::Node { return asset.nodes[i]; }))
            self(childNode);
    };

    for (const fastgltf::Node& node : scene.nodeIndices | std::views::transform([&](std::size_t i) -> fastgltf::Node { return asset.nodes[i]; }))
        processNode(node);

    registerAsset(
        std::nullopt,
        path.stem().string(),
        VAssetLocation(AssetLocation<Mesh>{path, 0}),
        dependentAssets | std::ranges::to<std::vector>()
    );
}

std::shared_future<void> AssetManager::loadAsset(AssetID assetId)
{
    assert(isValidAssetId(assetId));
    return std::visit([&]<ManagableAsset T>(AssetHandle<T>& handle) -> std::shared_future<void> {
        loadAsset<T>(assetId);
        return handle.voidFuture;
    },
    m_assetHandles.at(assetId));
}

void AssetManager::unloadAsset(AssetID assetId)
{
    assert(isValidAssetId(assetId));
    assert(std::visit([&]<ManagableAsset T>(AssetHandle<T>& handle) -> bool {
        return handle.future.valid();
    }, m_assetHandles.at(assetId)));
    assert(std::visit([&]<ManagableAsset T>(AssetHandle<T>& handle) -> bool {
        return handle.loadCount > 0;
    }, m_assetHandles.at(assetId)));

    std::visit([&]<ManagableAsset T>(AssetHandle<T>& handle) {
        handle.future.wait();
        handle.loadCount--;
        if (handle.loadCount == 0) {
            if constexpr (std::same_as<T, gfx::Texture>) {
                if (handle.loadStatus.load() == LoadStatus::loaded)
                    removeTexture(assetId);
            }
            handle.future = std::shared_future<std::shared_ptr<T>>();
            handle.voidFuture = std::shared_future<void>();
            handle.loadStatus.store(LoadStatus::unloaded);
        }
        unloadAssets(handle.dependentAssets);
    },
    m_assetHandles.at(assetId));
}

std::set<std::filesystem::path> AssetManager::assetContainerPaths() const
{
    std::scoped_lock lock(m_registeredAssetsMutex);
    return m_registeredAssets
        | std::views::keys
        | std::views::transform([](const VAssetLocation& vLocation) -> const std::filesystem::path& {
            return std::visit([]<typename T>(const AssetLocation<T>& location) -> const std::filesystem::path& {
                return location.containerPath;
            }, vLocation);
        })
        | std::ranges::to<std::set>();
}

AssetID AssetManager::assetId(const VAssetLocation& location) const
{
    std::scoped_lock lock(m_registeredAssetsMutex);
    assert(m_registeredAssets.contains(location));
    return m_registeredAssets.at(location);
}

std::string_view AssetManager::assetName(AssetID assetId) const
{
    assert(isValidAssetId(assetId));
    const VAssetHandle& vHandle = m_assetHandles.at(assetId);

    return std::visit([]<ManagableAsset T>(const AssetHandle<T>& handle) -> std::string_view {
        return handle.name;
    }, vHandle);
}

std::span<const AssetID> AssetManager::assetDependencies(AssetID assetId) const
{
    assert(isValidAssetId(assetId));
    const VAssetHandle& vHandle = m_assetHandles.at(assetId);

    return std::visit([]<ManagableAsset T>(const AssetHandle<T>& handle) -> std::span<const AssetID> {
        return handle.dependentAssets;
    }, vHandle);
}

std::optional<VAssetLocation> AssetManager::assetLocation(AssetID assetId) const
{
    assert(isValidAssetId(assetId));
    const VAssetHandle& vHandle = m_assetHandles.at(assetId);

    return std::visit([]<ManagableAsset T>(const AssetHandle<T>& handle) -> std::optional<VAssetLocation> {
        return handle.location;
    }, vHandle);
}

uint32_t AssetManager::assetLoadCount(AssetID assetId) const
{
    assert(isValidAssetId(assetId));
    const VAssetHandle& vHandle = m_assetHandles.at(assetId);

    return std::visit([]<ManagableAsset T>(const AssetHandle<T>& handle) -> uint32_t {
        return handle.loadCount;
    }, vHandle);
}

void AssetManager::setTexture(AssetID assetId, const std::shared_ptr<gfx::Texture>& texture)
{
    assert(isValidAssetId(assetId));
    assert(assetTypeIs<gfx::Texture>(assetId));
    assert(texture);

    if (std::shared_ptr<TextureTable> textureTable = m_textureTable.lock())
        textureTable->addTexture(assetId, texture);
}

void AssetManager::removeTexture(AssetID assetId)
{
    assert(isValidAssetId(assetId));
    assert(assetTypeIs<gfx::Texture>(assetId));

    if (std::shared_ptr<TextureTable> textureTable = m_textureTable.lock())
        textureTable->removeTexture(assetId);
}

void AssetManager::attachTextureTable(const std::shared_ptr<TextureTable>& textureTable)
{
    assert(textureTable);

    m_textureTable = textureTable;

    for (const auto& [assetId, vHandle] : m_assetHandles) {
        std::visit([&]<ManagableAsset T>(const AssetHandle<T>& handle) {
            if constexpr (std::same_as<T, gfx::Texture>) {
                if (handle.loadCount == 0 || handle.future.valid() == false)
                    return;
                if (handle.future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
                    return;
                textureTable->addTexture(assetId, handle.future.get());
            }
        }, vHandle);
    }
}

bool AssetManager::isRegistered(const VAssetLocation& location) const
{
    std::scoped_lock lock(m_registeredAssetsMutex);
    return m_registeredAssets.contains(location);
}

bool AssetManager::isValidAssetId(AssetID assetId) const
{
    return m_assetHandles.contains(assetId);
}

bool AssetManager::isAssetLoaded(AssetID assetId) const
{
    assert(isValidAssetId(assetId));
    const VAssetHandle& vHandle = m_assetHandles.at(assetId);

    return std::visit([]<ManagableAsset T>(const AssetHandle<T>& handle) -> bool {
        return handle.loadStatus.load() == LoadStatus::loaded;
    }, vHandle);
}

bool AssetManager::isAssetContainerLoaded(const std::filesystem::path& path) const
{
    std::lock_guard lock(m_assetContainersMutex);

    const auto it = m_assetContainers.find(path);
    return it != m_assetContainers.end() && it->second.expired() == false;
}

std::shared_ptr<AssetContainer> AssetManager::assetContainer(const std::filesystem::path& path)
{
    std::lock_guard lock(m_assetContainersMutex);

    if (auto it = m_assetContainers.find(path); it != m_assetContainers.end()) {
        if (std::shared_ptr<AssetContainer> container = it->second.lock())
            return container;
    }

    std::shared_ptr<AssetContainer> container;
    if (isImagePath(path))
        container = std::make_shared<ImageAssetContainer>(path);
    else if (isGltfPath(path))
        container = std::make_shared<GltfAssetContainer>(path);
    else
        throw std::runtime_error("unsupported asset container: " + path.string());

    m_assetContainers[path] = container;
    return container;
}

} // namespace GE
