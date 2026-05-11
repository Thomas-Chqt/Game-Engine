/*
 * ---------------------------------------------------
 * AssetManager.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/13 12:00:28
 * ---------------------------------------------------
 */

#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Mesh.hpp"

#include <Graphics/Device.hpp>
#include <Graphics/Enums.hpp>

#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/math.hpp>
#include <fastgltf/types.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image/stb_image.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <cstdint>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include <format>

namespace GE
{

namespace
{

namespace gltf = fastgltf;

std::shared_ptr<gfx::Buffer> newDeviceLocalBuffer(gfx::Device& device, gfx::CommandBuffer& commandBuffer, gfx::BufferUsage usage, const std::ranges::sized_range auto& data)
{
    std::shared_ptr<gfx::Buffer> deviceBuffer = device.newBuffer(gfx::Buffer::Descriptor{
        .size = sizeof(std::ranges::range_value_t<decltype(data)>) * data.size(),
        .usages = usage | gfx::BufferUsage::copyDestination,
        .storageMode = gfx::ResourceStorageMode::deviceLocal });
    assert(deviceBuffer);

    std::shared_ptr<gfx::Buffer> stagingBuffer = device.newBuffer(gfx::Buffer::Descriptor{
        .size = deviceBuffer->size(),
        .usages = gfx::BufferUsage::copySource,
        .storageMode = gfx::ResourceStorageMode::hostVisible });
    assert(stagingBuffer);

    std::ranges::copy(data, stagingBuffer->content<std::ranges::range_value_t<decltype(data)>>());
    commandBuffer.copyBufferToBuffer(stagingBuffer, deviceBuffer, deviceBuffer->size());
    return deviceBuffer;
}

std::shared_ptr<gfx::Texture> loadTexture(gfx::Device& device, const std::byte* bytes, uint32_t width, uint32_t height, gfx::CommandBuffer& commandBuffer)
{
    assert(bytes);
    std::shared_ptr<gfx::Texture> texture = device.newTexture(gfx::Texture::Descriptor{
        .type = gfx::TextureType::texture2d,
        .width = width,
        .height = height,
        .pixelFormat = gfx::PixelFormat::RGBA8Unorm,
        .usages = gfx::TextureUsage::copyDestination | gfx::TextureUsage::shaderRead,
        .storageMode = gfx::ResourceStorageMode::deviceLocal });
    assert(texture);

    std::shared_ptr<gfx::Buffer> stagingBuffer = device.newBuffer(gfx::Buffer::Descriptor{
        .size = static_cast<size_t>(width) * static_cast<size_t>(height) * pixelFormatSize(gfx::PixelFormat::RGBA8Unorm),
        .usages = gfx::BufferUsage::copySource,
        .storageMode = gfx::ResourceStorageMode::hostVisible });
    assert(stagingBuffer);

    std::memcpy(stagingBuffer->content<uint32_t>(), bytes, stagingBuffer->size());

    commandBuffer.beginBlitPass();
    commandBuffer.copyBufferToTexture(stagingBuffer, texture);
    commandBuffer.endBlitPass();

    return texture;
}

std::shared_ptr<gfx::Texture> loadTexture(gfx::Device& device, const std::filesystem::path& path, gfx::CommandBuffer& commandBuffer)
{
    using UniqueStbiUc = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;

    int width = 0;
    int height = 0;
    UniqueStbiUc bytes = UniqueStbiUc(stbi_load(path.string().c_str(), &width, &height, nullptr, STBI_rgb_alpha), stbi_image_free);
    if (!bytes)
        throw std::runtime_error("failed to load texture: " + path.string());
    return loadTexture(device, std::bit_cast<const std::byte*>(bytes.get()), static_cast<uint32_t>(width), static_cast<uint32_t>(height), commandBuffer);
}

Mesh loadMesh(gfx::Device& device, const std::filesystem::path& path, gfx::CommandBuffer& commandBuffer)
{
    assert(std::filesystem::is_regular_file(path));

    gltf::Expected<gltf::GltfDataBuffer> gltfDataBuffer = gltf::GltfDataBuffer::FromPath(path);
    if (gltfDataBuffer.error() != gltf::Error::None)
        throw std::runtime_error(std::format("{}: failed to load glTF: data error", path.string()));

    gltf::Parser parser;

    gltf::Expected<gltf::Asset> asset = parser.loadGltf(gltfDataBuffer.get(), path.parent_path(), gltf::Options::LoadExternalBuffers | gltf::Options::GenerateMeshIndices, gltf::Category::Asset | gltf::Category::Buffers | gltf::Category::BufferViews | gltf::Category::Accessors | gltf::Category::Meshes | gltf::Category::Nodes | gltf::Category::Scenes);
    if (asset.error() != gltf::Error::None)
        throw std::runtime_error(std::format("{}: failed to load glTF: asset error", path.string()));

    if(asset->scenes.empty())
        throw std::runtime_error(std::format("{}: failed to load glTF: no scene", path.string()));
    const gltf::Scene& scene = asset->defaultScene ? asset->scenes[*asset->defaultScene] : asset->scenes.front();

    Mesh mesh;

    auto addNode = [&](this auto&& self, std::vector<SubMesh>& dst, const gltf::Node& node, const glm::mat4x4& additionalTransform = glm::mat4x4(1.0f)) -> void
    {
        glm::mat4x4 transform;
        if (const gltf::math::fmat4x4* nodeTransform = std::get_if<gltf::math::fmat4x4>(&node.transform))
            transform = additionalTransform * glm::make_mat4x4(nodeTransform->data());
        else if (const gltf::TRS* nodeTRS = std::get_if<gltf::TRS>(&node.transform)) {
            auto matrix = glm::mat4x4(1.0f);
            matrix = glm::translate(matrix, glm::make_vec3(nodeTRS->translation.data()));
            matrix = glm::rotate(matrix, nodeTRS->rotation.x(), glm::vec3(1, 0, 0));
            matrix = glm::rotate(matrix, nodeTRS->rotation.y(), glm::vec3(0, 1, 0));
            matrix = glm::rotate(matrix, nodeTRS->rotation.z(), glm::vec3(0, 0, 1));
            matrix = glm::scale(matrix, glm::make_vec3(nodeTRS->scale.data()));
            transform = additionalTransform * matrix;
        }
        else std::unreachable();

        std::size_t dstSize = dst.size();
        if (gltf::Optional<std::size_t> meshIndex = node.meshIndex)
        {
            for (uint32_t i = 0; const gltf::Primitive& primitive : asset->meshes[*meshIndex].primitives)
            {
                if (primitive.type != gltf::PrimitiveType::Triangles)
                    throw std::runtime_error(std::format("{}: failed to load glTF: unsuported primitive type", path.string()));

                auto findAccessor = [&](std::string_view name) -> const gltf::Accessor* {
                    auto it = std::ranges::find_if(primitive.attributes, [&](const gltf::Attribute& attribute) -> bool { return attribute.name == name; });
                    return it == primitive.attributes.end() ? nullptr : &asset->accessors[it->accessorIndex];
                };

                const gltf::Accessor* posAccessor = findAccessor("POSITION");
                if (posAccessor == nullptr)
                    throw std::runtime_error(std::format("{}: failed to load glTF: pos accessor not found", path.string()));

                const gltf::Accessor* indicesAccessor = primitive.indicesAccessor.has_value() ? &asset->accessors[*primitive.indicesAccessor] : nullptr;
                if (indicesAccessor == nullptr)
                    throw std::runtime_error(std::format("{}: failed to load glTF: index accessor not found", path.string()));

                const gltf::Accessor* uvAccessor = findAccessor("TEXCOORD_0");
                const gltf::Accessor* normalAccessor = findAccessor("NORMAL");
                const gltf::Accessor* tangentAccessor = findAccessor("TANGENT");

                dst.emplace_back(SubMesh{
                    .name = std::format("{}{}", asset->meshes[*meshIndex].name, i++),
                    .transform = transform,
                    .vertexBuffer = newDeviceLocalBuffer(device, commandBuffer, gfx::BufferUsage::vertexBuffer, std::views::iota(std::size_t(0), posAccessor->count) | std::views::transform([&](std::size_t i) -> Vertex {
                        return Vertex{
                            .pos     = gltf::getAccessorElement<glm::vec3>(asset.get(), *posAccessor, i),
                            .uv      = uvAccessor      ? uvAccessor->count      > i ? gltf::getAccessorElement<glm::vec2>(asset.get(), *uvAccessor,      i) : glm::vec2{} : glm::vec2{},
                            .normal  = normalAccessor  ? normalAccessor->count  > i ? gltf::getAccessorElement<glm::vec3>(asset.get(), *normalAccessor,  i) : glm::vec3{} : glm::vec3{},
                            .tangent = tangentAccessor ? tangentAccessor->count > i ? gltf::getAccessorElement<glm::vec3>(asset.get(), *tangentAccessor, i) : glm::vec3{} : glm::vec3{}
                        };
                    })),
                    .indexBuffer = newDeviceLocalBuffer(device, commandBuffer, gfx::BufferUsage::indexBuffer, std::views::iota(std::size_t(0), indicesAccessor->count) | std::views::transform([&](std::size_t i) -> uint32_t {
                        return gltf::getAccessorElement<uint32_t>(asset.get(), *indicesAccessor, i);
                    })),
                });
            }
        }

        for (const gltf::Node& childNode : node.children | std::views::transform([&](std::size_t i) -> gltf::Node { return asset->nodes[i]; }))
        {
            if (dstSize == dst.size()) // no submeshes insterted
                self(dst, childNode, transform);
            else
                self(dst, childNode);
        }
    };

    commandBuffer.beginBlitPass();
    for (const gltf::Node& node :  scene.nodeIndices | std::views::transform([&](std::size_t i) -> gltf::Node { return asset->nodes[i]; }))
    {
        addNode(mesh.subMeshes, node);
    }
    commandBuffer.endBlitPass();

    return mesh;
}

Mesh loadBuiltInCube(gfx::Device& device, gfx::CommandBuffer& commandBuffer)
{
    constexpr auto vertices = std::to_array<Vertex>({
        { {-1, -1, -1}, {0, 1}, {-1,  0,  0}, { 0,  1,  0} },
        { {-1,  1, -1}, {1, 1}, {-1,  0,  0}, { 0,  1,  0} },
        { {-1,  1,  1}, {1, 0}, {-1,  0,  0}, { 0,  1,  0} },
        { {-1, -1,  1}, {0, 0}, {-1,  0,  0}, { 0,  1,  0} },
        { {-1, -1,  1}, {0, 1}, { 0,  0,  1}, { 0,  1,  1} },
        { {-1,  1,  1}, {1, 1}, { 0,  0,  1}, { 0,  1,  1} },
        { { 1,  1,  1}, {1, 0}, { 0,  0,  1}, { 0,  1,  1} },
        { { 1, -1,  1}, {0, 0}, { 0,  0,  1}, { 0,  1,  1} },
        { { 1, -1,  1}, {0, 1}, { 1,  0,  0}, { 0,  1,  0} },
        { { 1,  1,  1}, {1, 1}, { 1,  0,  0}, { 0,  1,  0} },
        { { 1,  1, -1}, {1, 0}, { 1,  0,  0}, { 0,  1,  0} },
        { { 1, -1, -1}, {0, 0}, { 1,  0,  0}, { 0,  1,  0} },
        { { 1, -1, -1}, {1, 0}, { 0,  0, -1}, { 0, -1, -1} },
        { { 1,  1, -1}, {0, 0}, { 0,  0, -1}, { 0, -1, -1} },
        { {-1,  1, -1}, {0, 1}, { 0,  0, -1}, { 0, -1, -1} },
        { {-1, -1, -1}, {1, 1}, { 0,  0, -1}, { 0, -1, -1} },
        { {-1, -1,  1}, {0, 1}, { 0, -1,  0}, { 1,  0,  0} },
        { { 1, -1,  1}, {1, 1}, { 0, -1,  0}, { 1,  0,  0} },
        { { 1, -1, -1}, {1, 0}, { 0, -1,  0}, { 1,  0,  0} },
        { {-1, -1, -1}, {0, 0}, { 0, -1,  0}, { 1,  0,  0} },
        { { 1,  1,  1}, {0, 1}, { 0,  1,  0}, {-1,  0,  0} },
        { {-1,  1,  1}, {1, 1}, { 0,  1,  0}, {-1,  0,  0} },
        { {-1,  1, -1}, {1, 0}, { 0,  1,  0}, {-1,  0,  0} },
        { { 1,  1, -1}, {0, 0}, { 0,  1,  0}, {-1,  0,  0} },
    });

    constexpr auto indices = std::to_array<uint32_t>({
         2,  1,  0,  3,  2,  0,
         6,  5,  4,  7,  6,  4,
        10,  9,  8, 11, 10,  8,
        14, 13, 12, 15, 14, 12,
        18, 17, 16, 19, 18, 16,
        22, 21, 20, 23, 22, 20
    });

    commandBuffer.beginBlitPass();
    auto mesh = Mesh {
        .subMeshes = std::vector<SubMesh> {
            {
                .name = "built_in_cube_submesh",
                .transform = glm::mat4(1.0f),
                .vertexBuffer = newDeviceLocalBuffer(device, commandBuffer, gfx::BufferUsage::vertexBuffer, vertices),
                .indexBuffer = newDeviceLocalBuffer(device, commandBuffer, gfx::BufferUsage::indexBuffer, indices),
                .subMeshes = {}
            }
        }
    };
    commandBuffer.endBlitPass();
    return mesh;
}

} // namespace

AssetManager::AssetManager(gfx::Device* device) : m_device(device)
{
    registerAsset(AssetPath<Mesh>(BUILT_IN_CUBE_PATH));
}

void AssetManager::registerAsset(const VAssetPath& vAssetPath)
{
    std::visit([&](const auto& assetPath) {
        using AssetType = typename std::remove_cvref_t<decltype(assetPath)>::AssetType;
        auto [it, inserted] = m_registredAsset.try_emplace(vAssetPath, std::in_place_type<AssetHandle<AssetType>>);
        if (inserted)
        {
            assert(assetPath.path == BUILT_IN_CUBE_PATH || std::filesystem::is_regular_file(assetPath));
            AssetHandle<AssetType>& handle = std::get<AssetHandle<AssetType>>(it->second);
            if constexpr (std::is_same_v<AssetType, Mesh>) {
                if (assetPath.path == BUILT_IN_CUBE_PATH) {
                    handle.loader = [device=m_device](gfx::CommandBuffer& commandBuffer) -> std::shared_ptr<Mesh> {
                        return std::make_shared<Mesh>(::GE::loadBuiltInCube(*device, commandBuffer));
                    };
                }
                else {
                    handle.loader = [device=m_device, path=assetPath](gfx::CommandBuffer& commandBuffer) {
                        return std::make_shared<Mesh>(loadMesh(*device, path, commandBuffer));
                    };
                }
            }
            else if constexpr (std::is_same_v<AssetType, gfx::Texture>) {
                handle.loader = [device=m_device, path=assetPath](gfx::CommandBuffer& commandBuffer) {
                    return loadTexture(*device, path, commandBuffer);
                };
            }
            else std::unreachable();
            if (assetPath.path == BUILT_IN_CUBE_PATH) {
                m_assetMetadatas.try_emplace(vAssetPath, AssetMetadata{ .name="built_in_cube" });
            }
            else {
                m_assetMetadatas.try_emplace(vAssetPath, AssetMetadata{ .name=assetPath.path.stem() });
            }
        }
    },
    vAssetPath);
}

void AssetManager::unloadAssetHandle(VAssetHandle& vHandle)
{
    std::visit([](auto& handle) {
        if (handle.future.valid())
            handle.future.wait(); // dont need to propagate errors
        const uint32_t previousRefCount = handle.refCount.fetch_sub(1);
        assert(previousRefCount > 0);
        if (previousRefCount == 1)
        {
            auto expected = AssetHandleLoadingStatus::loaded;
            bool res = handle.status.compare_exchange_strong(expected, AssetHandleLoadingStatus::unloaded);
            assert(res);
            handle.asset.reset();
        }
    },
    vHandle);
}

AssetManager::~AssetManager()
{
    const auto destroyAssetHandle = [](auto& handle) {
        if (handle.future.valid())
            handle.future.wait(); // dont need to propagate errors
        handle.refCount.store(0);
        handle.status.store(AssetHandleLoadingStatus::unloaded);
        handle.asset.reset();
    };

    for (auto& [_, handle] : m_registredAsset)
        std::visit(destroyAssetHandle, handle);
}

} // namespace GE
