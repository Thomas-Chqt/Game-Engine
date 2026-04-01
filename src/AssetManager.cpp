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

#include <array>
#include <stb_image/stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/types.h>

#include <cassert>
#include <cstdint>
#include <ranges>
#include <bit>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef __cpp_lib_containers_ranges
    #define APPEND_RANGE(self, range) self.append_range(range)
#else
    #define APPEND_RANGE(self, range) self.insert(self.end(), range.cbegin(), range.cend())
#endif

constexpr unsigned int POST_PROCESSING_FLAGS = aiProcess_CalcTangentSpace
                                               | aiProcess_JoinIdenticalVertices
                                               | aiProcess_Triangulate
                                               | aiProcess_GenNormals
                                               | aiProcess_OptimizeMeshes
                                               | aiProcess_FlipUVs;

namespace
{

static inline glm::mat4x4 toGlmMat4(const aiMatrix4x4& from)
{
    glm::mat4x4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

} // namespace

namespace GE
{

AssetManager::AssetManager(gfx::Device* device)
    : m_device(device)
    , m_builtInCubeHandle(std::in_place_type<AssetHandle<Mesh>>)
{
    std::get<AssetHandle<Mesh>>(m_builtInCubeHandle).loader = [device=m_device](gfx::CommandBuffer& commandBuffer) -> std::shared_ptr<Mesh> {
        return std::make_shared<Mesh>(loadBuiltInCube(*device, commandBuffer));
    };
}

void AssetManager::registerAsset(const VAssetPath& vAssetPath)
{
    std::visit([&](const auto& assetPath) {
        using AssetType = typename std::remove_cvref_t<decltype(assetPath)>::AssetType;
        auto [it, inserted] = m_handles.try_emplace(vAssetPath, std::in_place_type<AssetHandle<AssetType>>);
        if (inserted)
        {
            AssetHandle<AssetType>& handle = std::get<AssetHandle<AssetType>>(it->second);
            if constexpr (std::is_same_v<AssetType, Mesh>) {
                handle.loader = [device=m_device, path=assetPath](gfx::CommandBuffer& commandBuffer) {
                    return std::make_shared<Mesh>(loadMesh(*device, path, commandBuffer));
                };
            }
            else if constexpr (std::is_same_v<AssetType, gfx::Texture>) {
                handle.loader = [device=m_device, path=assetPath](gfx::CommandBuffer& commandBuffer) {
                    return loadTexture(*device, path, commandBuffer);
                };
            }
            else std::unreachable();
        }
    },
    vAssetPath);
}

void AssetManager::unloadAssetHandle(VAssetHandle& vHandle)
{
    std::visit([](auto& handle) {
        handle.future.wait(); // dont need to propagate errors
        auto expected = AssetHandleLoadingStatus::loaded;
        if (handle.status.compare_exchange_strong(expected, AssetHandleLoadingStatus::unloaded))
            handle.asset.reset();
    },
    vHandle);
}

AssetManager::~AssetManager()
{
    for (auto& [_, handle] : m_handles)
        unloadAssetHandle(handle);
}

Mesh AssetManager::loadMesh(gfx::Device& device, const std::filesystem::path& path, gfx::CommandBuffer& commandBuffer)
{
    assert(std::filesystem::is_regular_file(path));

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path.string(), POST_PROCESSING_FLAGS);
    if (scene == nullptr)
        throw std::runtime_error("fail to load the model using assimp");

    const auto aiMeshToSubMesh = [&](aiMesh* aiMesh) {
        const auto aiVtxToVtx = [aiMesh](uint32_t i) -> Vertex {
            return Vertex{
                .pos = glm::vec3(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z),
                .uv = aiMesh->mTextureCoords[0] != nullptr ? glm::vec2(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y) : glm::vec2(0.0f),
                .normal = aiMesh->mNormals != nullptr ? glm::vec3(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z) : glm::vec3(0.0f),
                .tangent = aiMesh->mTangents != nullptr ? glm::vec3(aiMesh->mTangents[i].x, aiMesh->mTangents[i].y, aiMesh->mTangents[i].z) : glm::vec3(0.0f)
            };
        };
        const auto aiVtxToIdx = [aiMesh](uint32_t i) -> uint32_t {
            return aiMesh->mFaces[i / 3].mIndices[i % 3];
        };
        return SubMesh{
            .name = aiMesh->mName.C_Str(),
            .transform = glm::mat4x4(1.0f),
            .vertexBuffer = newDeviceLocalBuffer(device, commandBuffer, gfx::BufferUsage::vertexBuffer, std::views::iota(0u, aiMesh->mNumVertices) | std::views::transform(std::move(aiVtxToVtx))),
            .indexBuffer = newDeviceLocalBuffer(device, commandBuffer, gfx::BufferUsage::indexBuffer, std::views::iota(0u, aiMesh->mNumFaces * 3) | std::views::transform(std::move(aiVtxToIdx))),
            // .material = materials[aiMesh->mMaterialIndex],
            .subMeshes = {}
        };
    };

    commandBuffer.beginBlitPass();
    auto flatSubMeshes = std::span(scene->mMeshes, scene->mNumMeshes) | std::views::transform(std::move(aiMeshToSubMesh)) | std::ranges::to<std::vector>();
    commandBuffer.endBlitPass();

    std::function<void(std::vector<SubMesh>&, aiNode*, glm::mat4x4)> addNode = [&](std::vector<SubMesh>& dest, aiNode* aiNode, glm::mat4x4 additionalTransform) {
        glm::mat4x4 transform = additionalTransform * toGlmMat4(aiNode->mTransformation);

        const auto nodeMeshToSubmesh = [&](uint32_t i) -> SubMesh {
            SubMesh submesh = flatSubMeshes[i];
            submesh.transform = transform;
            return submesh;
        };
        auto subMeshes = std::span(aiNode->mMeshes, aiNode->mNumMeshes) | std::views::transform(std::move(nodeMeshToSubmesh));

        for (auto* node : std::span(aiNode->mChildren, aiNode->mNumChildren))
        {
            if (subMeshes.empty())
                addNode(dest, node, transform);
            else
            {
                std::vector<SubMesh> subDest;
                addNode(subDest, node, glm::mat4x4(1.0F));
                APPEND_RANGE(subMeshes.front().subMeshes, subDest);
            }
        }
        APPEND_RANGE(dest, subMeshes);
    };

    Mesh mesh = {
        .name = scene->mRootNode->mName.C_Str(),
        .subMeshes = std::span(scene->mRootNode->mMeshes, scene->mRootNode->mNumMeshes)
                     | std::views::transform([&](uint32_t i) -> SubMesh {
                           SubMesh submesh = flatSubMeshes[i];
                           submesh.transform = glm::mat4x4(1.0F);
                           return submesh;
                       })
                     | std::ranges::to<std::vector>()
    };

    for (auto* node : std::span(scene->mRootNode->mChildren, scene->mRootNode->mNumChildren))
        addNode(mesh.subMeshes, node, glm::mat4x4(1.0F));

    return mesh;
}

std::shared_ptr<gfx::Texture> AssetManager::loadTexture(gfx::Device& device, const std::filesystem::path& path, gfx::CommandBuffer& commandBuffer)
{
    using UniqueStbiUc = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;

    int width = 0;
    int height = 0;
    UniqueStbiUc bytes = UniqueStbiUc(stbi_load(path.string().c_str(), &width, &height, nullptr, STBI_rgb_alpha), stbi_image_free);
    if (!bytes)
        throw std::runtime_error("failed to load texture: " + path.string());
    return loadTexture(device, std::bit_cast<const std::byte*>(bytes.get()), static_cast<uint32_t>(width), static_cast<uint32_t>(height), commandBuffer);
}

std::shared_ptr<gfx::Texture> AssetManager::loadTexture(gfx::Device& device, const std::byte* bytes, uint32_t width, uint32_t height, gfx::CommandBuffer& commandBuffer)
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

Mesh AssetManager::loadBuiltInCube(gfx::Device& device, gfx::CommandBuffer& commandBuffer)
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
        .name = "built_in_cube",
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

} // namespace GE
