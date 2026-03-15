/*
 * ---------------------------------------------------
 * AssetManager.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/13 12:00:28
 * ---------------------------------------------------
 */

#include "Game-Engine/AssetManager.hpp"

#include <Graphics/Device.hpp>
#include <Graphics/Enums.hpp>

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
#include <utility>

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
{
}

void AssetManager::unloadAsset(AssetID assetId)
{
    auto it = m_assets.find(assetId);
    assert(it != m_assets.end());

    auto visitor = [](auto&& handle) {
        handle.future.wait(); // dont need to propagate errors
        auto expected = LoadingStatus::loaded;
        if (handle.status.compare_exchange_strong(expected, LoadingStatus::unloaded))
            handle.asset.reset();
    };
    std::visit(std::move(visitor), it->second);
}

AssetManager::~AssetManager()
{
    for (auto& [id, _] : m_assets)
        unloadAsset(id);
}

Mesh AssetManager::loadMesh(const std::filesystem::path& path, gfx::CommandBuffer& commandBuffer)
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
            .vertexBuffer = newDeviceLocalBuffer(commandBuffer, gfx::BufferUsage::vertexBuffer, std::views::iota(0u, aiMesh->mNumVertices) | std::views::transform(std::move(aiVtxToVtx))),
            .indexBuffer = newDeviceLocalBuffer(commandBuffer, gfx::BufferUsage::indexBuffer, std::views::iota(0u, aiMesh->mNumFaces * 3) | std::views::transform(std::move(aiVtxToIdx))),
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

std::shared_ptr<gfx::Texture> AssetManager::loadTexture(const std::filesystem::path& path, gfx::CommandBuffer& commandBuffer)
{
    using UniqueStbiUc = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;

    int width = 0;
    int height = 0;
    UniqueStbiUc bytes = UniqueStbiUc(stbi_load(path.string().c_str(), &width, &height, nullptr, STBI_rgb_alpha), stbi_image_free);
    if (!bytes)
        throw std::runtime_error("failed to load texture: " + path.string());
    return loadTexture(std::bit_cast<const std::byte*>(bytes.get()), static_cast<uint32_t>(width), static_cast<uint32_t>(height), commandBuffer);
}

std::shared_ptr<gfx::Texture> AssetManager::loadTexture(const std::byte* bytes, uint32_t width, uint32_t height, gfx::CommandBuffer& commandBuffer)
{
    assert(bytes);
    std::shared_ptr<gfx::Texture> texture = m_device->newTexture(gfx::Texture::Descriptor{
        .type = gfx::TextureType::texture2d,
        .width = width,
        .height = height,
        .pixelFormat = gfx::PixelFormat::RGBA8Unorm,
        .usages = gfx::TextureUsage::copyDestination | gfx::TextureUsage::shaderRead,
        .storageMode = gfx::ResourceStorageMode::deviceLocal });
    assert(texture);

    std::shared_ptr<gfx::Buffer> stagingBuffer = m_device->newBuffer(gfx::Buffer::Descriptor{
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

} // namespace GE
