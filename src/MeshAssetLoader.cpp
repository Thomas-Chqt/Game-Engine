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
#include <cassert>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/math.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <format>
#include <map>
#include <memory>
#include <ranges>
#include <utility>

namespace gltf = fastgltf;

namespace GE
{

constexpr auto vertices = std::to_array<Vertex>({
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

constexpr auto indices = std::to_array<uint32_t>({
     2,  1,  0,  3,  2,  0,
     6,  5,  4,  7,  6,  4,
    10,  9,  8, 11, 10,  8,
    14, 13, 12, 15, 14, 12,
    18, 17, 16, 19, 18, 16,
    22, 21, 20, 23, 22, 20
});

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
    assert(m_assetManager);
    std::shared_ptr<AssetContainer> container = m_assetManager->assetContainer(location.containerPath);
    auto* gltfContainer = dynamic_cast<GltfAssetContainer*>(container.get());
    assert(gltfContainer);
    assert(location.index == 0);

    const gltf::Asset& asset = gltfContainer->asset();
    if (asset.scenes.empty())
        throw std::runtime_error(std::format("{}: failed to load glTF: no scene", location.containerPath.string()));
    const gltf::Scene& scene = asset.defaultScene ? asset.scenes[*asset.defaultScene] : asset.scenes.front();

    auto mesh = std::make_shared<Mesh>();
    std::map<std::size_t, std::shared_ptr<Material>> materialCache;
    std::shared_ptr<Material> defaultMaterial;
    auto getDefaultMaterial = [&]() -> std::shared_ptr<Material> {
        if (defaultMaterial == nullptr) {
            defaultMaterial = std::make_shared<Material>(Material{
                .diffuseColor = glm::vec4(1.0f),
                .diffuseTexture = BUILT_IN_WHITE_TEXTURE_ID,
                .specularColor = glm::vec3(0.0f),
                .shininess = 0.0f
            });
        }
        return defaultMaterial;
    };

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

            it->second = std::make_shared<Material>(Material{
                .diffuseColor = glm::make_vec4(gltfMaterial.pbrData.baseColorFactor.data()),
                .diffuseTexture = diffuseTexture,
                .specularColor = glm::vec3(0.0f),
                .shininess = 0.0f
            });
        }
        return it->second;
    };

    auto addNode = [&](this auto&& self, std::vector<SubMesh>& dst, const gltf::Node& node, const glm::mat4x4& additionalTransform = glm::mat4x4(1.0f)) -> void
    {
        glm::mat4x4 transform;
        if (const gltf::math::fmat4x4* nodeTransform = std::get_if<gltf::math::fmat4x4>(&node.transform))
            transform = additionalTransform * glm::make_mat4x4(nodeTransform->data());
        else if (const gltf::TRS* nodeTRS = std::get_if<gltf::TRS>(&node.transform))
        {
            const glm::vec3 translation = glm::make_vec3(nodeTRS->translation.data());
            const glm::vec3 scale = glm::make_vec3(nodeTRS->scale.data());
            const glm::quat rotation(nodeTRS->rotation.w(), nodeTRS->rotation.x(), nodeTRS->rotation.y(), nodeTRS->rotation.z());
            const glm::mat4x4 matrix = glm::translate(glm::mat4x4(1.0f), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4x4(1.0f), scale);
            transform = additionalTransform * matrix;
        }
        else
            std::unreachable();

        std::size_t dstSize = dst.size();
        if (gltf::Optional<std::size_t> meshIndex = node.meshIndex)
        {
            for (uint32_t i = 0; const gltf::Primitive& primitive : asset.meshes[*meshIndex].primitives)
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

                dst.emplace_back(SubMesh{
                    .name = std::format("{}{}", asset.meshes[*meshIndex].name, i++),
                    .transform = transform,
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
                    .material = primitive.materialIndex.has_value() ? getMaterial(*primitive.materialIndex) : getDefaultMaterial(),
                });
            }
        }

        for (const gltf::Node& childNode : node.children | std::views::transform([&](std::size_t i) -> gltf::Node { return asset.nodes[i]; }))
        {
            if (dstSize == dst.size())
                self(dst, childNode, transform);
            else
                self(dst, childNode);
        }
    };

    commandBuffer.beginBlitPass();
    for (const gltf::Node& node : scene.nodeIndices | std::views::transform([&](std::size_t i) -> gltf::Node { return asset.nodes[i]; }))
        addNode(mesh->subMeshes, node);
    commandBuffer.endBlitPass();

    return mesh;
}

std::shared_ptr<Mesh> AssetLoader<Mesh>::load(const BuiltInMesh& builtInMesh, gfx::CommandBuffer& commandBuffer) const
{
    switch (builtInMesh)
    {
    case BuiltInMesh::cube:
        commandBuffer.beginBlitPass();
        auto mesh = std::make_shared<Mesh>(Mesh{
            .subMeshes = std::vector<SubMesh>{{
                .name = "built_in_cube_submesh",
                .transform = glm::mat4(1.0f),
                .vertexBuffer = newDeviceLocalBuffer(*m_device, commandBuffer, gfx::BufferUsage::vertexBuffer, vertices),
                .indexBuffer = newDeviceLocalBuffer(*m_device, commandBuffer, gfx::BufferUsage::indexBuffer, indices),
                .material = std::make_shared<Material>(Material{
                    .diffuseColor = glm::vec4(1.0f),
                    .diffuseTexture = BUILT_IN_WHITE_TEXTURE_ID,
                    .specularColor = glm::vec3(0.0f),
                    .shininess = 0.0f
                }),
                .subMeshes = {}
            }}
        });
        commandBuffer.endBlitPass();
        return mesh;
        break;
    }
}

} // namespace GE
