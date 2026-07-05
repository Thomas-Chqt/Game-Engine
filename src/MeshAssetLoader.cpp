/*
 * ---------------------------------------------------
 * MeshAssetLoader.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/MeshAssetLoader.hpp"

#include "Game-Engine/AssetContainer.hpp"
#include "Game-Engine/AssetLoader.hpp"
#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Material.hpp"
#include "Game-Engine/Mesh.hpp"

#include <Graphics/Device.hpp>
#include <mutex>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#include <format>
#include <map>
#include <memory>
#include <ranges>
#include <string_view>
#include <utility>
#include <source_location>
#include <cassert>
#include <filesystem>
#include <source_location>

namespace gltf = fastgltf;

namespace GE
{

namespace
{

constexpr auto cubeVertices = std::to_array<Vertex>({
    { .pos={-1, -1, -1}, .uv={0, 1}, .normal={-1,  0,  0}, .tangent={ 0,  1,  0, 1} },
    { .pos={-1,  1, -1}, .uv={1, 1}, .normal={-1,  0,  0}, .tangent={ 0,  1,  0, 1} },
    { .pos={-1,  1,  1}, .uv={1, 0}, .normal={-1,  0,  0}, .tangent={ 0,  1,  0, 1} },
    { .pos={-1, -1,  1}, .uv={0, 0}, .normal={-1,  0,  0}, .tangent={ 0,  1,  0, 1} },
    { .pos={-1, -1,  1}, .uv={0, 1}, .normal={ 0,  0,  1}, .tangent={ 0,  1,  1, 1} },
    { .pos={-1,  1,  1}, .uv={1, 1}, .normal={ 0,  0,  1}, .tangent={ 0,  1,  1, 1} },
    { .pos={ 1,  1,  1}, .uv={1, 0}, .normal={ 0,  0,  1}, .tangent={ 0,  1,  1, 1} },
    { .pos={ 1, -1,  1}, .uv={0, 0}, .normal={ 0,  0,  1}, .tangent={ 0,  1,  1, 1} },
    { .pos={ 1, -1,  1}, .uv={0, 1}, .normal={ 1,  0,  0}, .tangent={ 0,  1,  0, 1} },
    { .pos={ 1,  1,  1}, .uv={1, 1}, .normal={ 1,  0,  0}, .tangent={ 0,  1,  0, 1} },
    { .pos={ 1,  1, -1}, .uv={1, 0}, .normal={ 1,  0,  0}, .tangent={ 0,  1,  0, 1} },
    { .pos={ 1, -1, -1}, .uv={0, 0}, .normal={ 1,  0,  0}, .tangent={ 0,  1,  0, 1} },
    { .pos={ 1, -1, -1}, .uv={1, 0}, .normal={ 0,  0, -1}, .tangent={ 0, -1, -1, 1} },
    { .pos={ 1,  1, -1}, .uv={0, 0}, .normal={ 0,  0, -1}, .tangent={ 0, -1, -1, 1} },
    { .pos={-1,  1, -1}, .uv={0, 1}, .normal={ 0,  0, -1}, .tangent={ 0, -1, -1, 1} },
    { .pos={-1, -1, -1}, .uv={1, 1}, .normal={ 0,  0, -1}, .tangent={ 0, -1, -1, 1} },
    { .pos={-1, -1,  1}, .uv={0, 1}, .normal={ 0, -1,  0}, .tangent={ 1,  0,  0, 1} },
    { .pos={ 1, -1,  1}, .uv={1, 1}, .normal={ 0, -1,  0}, .tangent={ 1,  0,  0, 1} },
    { .pos={ 1, -1, -1}, .uv={1, 0}, .normal={ 0, -1,  0}, .tangent={ 1,  0,  0, 1} },
    { .pos={-1, -1, -1}, .uv={0, 0}, .normal={ 0, -1,  0}, .tangent={ 1,  0,  0, 1} },
    { .pos={ 1,  1,  1}, .uv={0, 1}, .normal={ 0,  1,  0}, .tangent={-1,  0,  0, 1} },
    { .pos={-1,  1,  1}, .uv={1, 1}, .normal={ 0,  1,  0}, .tangent={-1,  0,  0, 1} },
    { .pos={-1,  1, -1}, .uv={1, 0}, .normal={ 0,  1,  0}, .tangent={-1,  0,  0, 1} },
    { .pos={ 1,  1, -1}, .uv={0, 0}, .normal={ 0,  1,  0}, .tangent={-1,  0,  0, 1} },
});

constexpr auto cubeIndices = std::to_array<uint32_t>({
     2,  1,  0,  3,  2,  0,
     6,  5,  4,  7,  6,  4,
    10,  9,  8, 11, 10,  8,
    14, 13, 12, 15, 14, 12,
    18, 17, 16, 19, 18, 16,
    22, 21, 20, 23, 22, 20
});

std::shared_ptr<Material> defaultMaterial()
{
    static std::weak_ptr<Material> weakMaterial;
    static std::mutex mutex;
    std::scoped_lock lock(mutex);

    std::shared_ptr<Material> material = weakMaterial.lock();
    if (material == nullptr) {
        material = std::make_shared<Material>(Material{
            .diffuseColor = glm::vec4(1.0f),
            .diffuseTexture = BUILT_IN_WHITE_TEXTURE_ID,
            .specularColor = glm::vec3(0.0f),
            .emissiveColor = glm::vec3(0.0f),
            .emissiveTexture = BUILT_IN_WHITE_TEXTURE_ID,
            .shininess = 0.0f
        });
        weakMaterial = material;
    }
    return material;
}

}

AssetLoader<Mesh>::AssetLoader(gfx::Device* device, AssetManager* assetManager, const AssetLocation<Mesh>& location)
    : AssetLoaderBase<Mesh>(device, assetManager)
    , m_source(location)
{
}

AssetLoader<Mesh>::AssetLoader(gfx::Device* device, AssetManager* assetManager, BuiltInMesh builtInMesh)
    : AssetLoaderBase<Mesh>(device, assetManager)
    , m_source(builtInMesh)
{
}

std::shared_ptr<Mesh> AssetLoader<Mesh>::load(gfx::CommandBuffer& commandBuffer) const
{
    return std::visit([&](const auto& source) { return load(source, commandBuffer); }, m_source);
}

std::shared_ptr<Mesh> AssetLoader<Mesh>::load(const AssetLocation<Mesh>& location, gfx::CommandBuffer& commandBuffer) const
{
    ZoneScopedN(std::source_location::current().function_name());

    assert(m_assetManager);
    std::shared_ptr<AssetContainer> container = m_assetManager->assetContainer(location.containerPath);
    auto* gltfContainer = dynamic_cast<GltfAssetContainer*>(container.get());
    assert(gltfContainer);

    const gltf::Asset& asset = gltfContainer->asset();
    const gltf::Mesh& gltfMesh = asset.meshes.at(location.index);

    auto mesh = std::make_shared<Mesh>();
    std::map<std::size_t, std::shared_ptr<Material>> materialCache;

    auto getMaterial = [&](std::size_t materialIndex) -> std::shared_ptr<Material> {
        auto [it, inserted] = materialCache.try_emplace(materialIndex);
        if (inserted) {
            const gltf::Material& gltfMaterial = asset.materials.at(materialIndex);
            AssetID diffuseTexture = BUILT_IN_WHITE_TEXTURE_ID;
            if (gltfMaterial.pbrData.baseColorTexture.has_value()) {
                const AssetLocation<gfx::Texture> diffuseTextureLocation{
                    .containerPath = location.containerPath,
                    .index = gltfMaterial.pbrData.baseColorTexture->textureIndex
                };
                diffuseTexture = m_assetManager->assetId(VAssetLocation(diffuseTextureLocation));
            }

            AssetID emissiveTexture = BUILT_IN_WHITE_TEXTURE_ID;
            if (gltfMaterial.emissiveTexture.has_value()) {
                const AssetLocation<gfx::Texture> emissiveTextureLocation{
                    .containerPath = location.containerPath,
                    .index = gltfMaterial.emissiveTexture->textureIndex
                };
                emissiveTexture = m_assetManager->assetId(VAssetLocation(emissiveTextureLocation));
            }

            it->second = std::make_shared<Material>(Material{
                .diffuseColor = glm::vec4(
                    glm::make_vec3(gltfMaterial.pbrData.baseColorFactor.data()),
                    gltfMaterial.alphaMode == gltf::AlphaMode::Opaque
                        ? 1.0f
                        : gltfMaterial.pbrData.baseColorFactor.w()),
                .diffuseTexture = diffuseTexture,
                .specularColor = glm::vec3(0.0f),
                .emissiveColor = glm::make_vec3(gltfMaterial.emissiveFactor.data()),
                .emissiveTexture = emissiveTexture,
                .shininess = 0.0f
            });
        }
        return it->second;
    };

    commandBuffer.beginBlitPass();
    for (uint32_t i = 0; const gltf::Primitive& primitive : gltfMesh.primitives)
    {
        if (primitive.type != gltf::PrimitiveType::Triangles)
            throw std::runtime_error(std::format("{}: failed to load glTF: unsuported primitive type", location.containerPath.string()));

        auto findAccessor = [&](std::string_view name) -> const gltf::Accessor* {
            auto it = std::ranges::find_if(primitive.attributes, [&](const gltf::Attribute& attribute) -> bool { return attribute.name == name; });
            return it == primitive.attributes.end() ? nullptr : &asset.accessors[it->accessorIndex];
        };

        const gltf::Accessor* posAccessor = findAccessor("POSITION");
        if (posAccessor == nullptr)
            throw std::runtime_error(std::format("{}: failed to load glTF: pos accessor not found", location.containerPath.string()));

        const gltf::Accessor* indicesAccessor = primitive.indicesAccessor.has_value() ? &asset.accessors[*primitive.indicesAccessor] : nullptr;
        if (indicesAccessor == nullptr)
            throw std::runtime_error(std::format("{}: failed to load glTF: index accessor not found", location.containerPath.string()));

        const gltf::Accessor* uvAccessor = findAccessor("TEXCOORD_0");
        const gltf::Accessor* normalAccessor = findAccessor("NORMAL");
        const gltf::Accessor* tangentAccessor = findAccessor("TANGENT");

        mesh->subMeshes.emplace_back(SubMesh{
            .name = std::format("{}{}", gltfMesh.name, i++),
            .vertexBuffer = this->newDeviceLocalBuffer(*m_device, commandBuffer, gfx::BufferUsage::vertexBuffer, std::views::iota(std::size_t(0), posAccessor->count) | std::views::transform([&](std::size_t i) -> Vertex {
                return Vertex{
                    .pos = gltf::getAccessorElement<glm::vec3>(asset, *posAccessor, i),
                    .uv = uvAccessor ? uvAccessor->count > i ? gltf::getAccessorElement<glm::vec2>(asset, *uvAccessor, i) : glm::vec2{} : glm::vec2{},
                    .normal = normalAccessor ? normalAccessor->count > i ? gltf::getAccessorElement<glm::vec3>(asset, *normalAccessor, i) : glm::vec3{} : glm::vec3{},
                    .tangent = tangentAccessor ? tangentAccessor->count > i ? gltf::getAccessorElement<glm::vec4>(asset, *tangentAccessor, i) : glm::vec4{} : glm::vec4{}
                };
            })),
            .indexBuffer = this->newDeviceLocalBuffer(*m_device, commandBuffer, gfx::BufferUsage::indexBuffer, std::views::iota(std::size_t(0), indicesAccessor->count) | std::views::transform([&](std::size_t i) -> uint32_t {
                return gltf::getAccessorElement<uint32_t>(asset, *indicesAccessor, i);
            })),
            .material = primitive.materialIndex.has_value() ? getMaterial(*primitive.materialIndex) : defaultMaterial(),
        });
    }
    commandBuffer.endBlitPass();

    return mesh;
}

std::shared_ptr<Mesh> AssetLoader<Mesh>::load(const BuiltInMesh& builtInMesh, gfx::CommandBuffer& commandBuffer) const
{
    ZoneScopedN(std::source_location::current().function_name());

    switch (builtInMesh)
    {
    case BuiltInMesh::cube:
        commandBuffer.beginBlitPass();
        auto mesh = std::make_shared<Mesh>(Mesh{
            .subMeshes = std::vector<SubMesh>{{
                .name = "built_in_cube_submesh",
                .vertexBuffer = newDeviceLocalBuffer(*m_device, commandBuffer, gfx::BufferUsage::vertexBuffer, cubeVertices),
                .indexBuffer = newDeviceLocalBuffer(*m_device, commandBuffer, gfx::BufferUsage::indexBuffer, cubeIndices),
                .material = defaultMaterial()
            }}
        });
        commandBuffer.endBlitPass();
        return mesh;
        break;
    }
}

} // namespace GE
