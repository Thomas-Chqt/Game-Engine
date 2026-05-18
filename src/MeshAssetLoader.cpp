/*
 * ---------------------------------------------------
 * MeshAssetLoader.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/MeshAssetLoader.hpp"

#include <Graphics/Device.hpp>

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
#include <memory>
#include <utility>
#include <ranges>

namespace gltf = fastgltf;

namespace GE
{

constexpr auto vertices = std::to_array<Vertex>({
    { .pos={-1, -1, -1}, .uv={0, 1}, .normal={-1,  0,  0}, .tangent={ 0,  1,  0} },
    { .pos={-1,  1, -1}, .uv={1, 1}, .normal={-1,  0,  0}, .tangent={ 0,  1,  0} },
    { .pos={-1,  1,  1}, .uv={1, 0}, .normal={-1,  0,  0}, .tangent={ 0,  1,  0} },
    { .pos={-1, -1,  1}, .uv={0, 0}, .normal={-1,  0,  0}, .tangent={ 0,  1,  0} },
    { .pos={-1, -1,  1}, .uv={0, 1}, .normal={ 0,  0,  1}, .tangent={ 0,  1,  1} },
    { .pos={-1,  1,  1}, .uv={1, 1}, .normal={ 0,  0,  1}, .tangent={ 0,  1,  1} },
    { .pos={ 1,  1,  1}, .uv={1, 0}, .normal={ 0,  0,  1}, .tangent={ 0,  1,  1} },
    { .pos={ 1, -1,  1}, .uv={0, 0}, .normal={ 0,  0,  1}, .tangent={ 0,  1,  1} },
    { .pos={ 1, -1,  1}, .uv={0, 1}, .normal={ 1,  0,  0}, .tangent={ 0,  1,  0} },
    { .pos={ 1,  1,  1}, .uv={1, 1}, .normal={ 1,  0,  0}, .tangent={ 0,  1,  0} },
    { .pos={ 1,  1, -1}, .uv={1, 0}, .normal={ 1,  0,  0}, .tangent={ 0,  1,  0} },
    { .pos={ 1, -1, -1}, .uv={0, 0}, .normal={ 1,  0,  0}, .tangent={ 0,  1,  0} },
    { .pos={ 1, -1, -1}, .uv={1, 0}, .normal={ 0,  0, -1}, .tangent={ 0, -1, -1} },
    { .pos={ 1,  1, -1}, .uv={0, 0}, .normal={ 0,  0, -1}, .tangent={ 0, -1, -1} },
    { .pos={-1,  1, -1}, .uv={0, 1}, .normal={ 0,  0, -1}, .tangent={ 0, -1, -1} },
    { .pos={-1, -1, -1}, .uv={1, 1}, .normal={ 0,  0, -1}, .tangent={ 0, -1, -1} },
    { .pos={-1, -1,  1}, .uv={0, 1}, .normal={ 0, -1,  0}, .tangent={ 1,  0,  0} },
    { .pos={ 1, -1,  1}, .uv={1, 1}, .normal={ 0, -1,  0}, .tangent={ 1,  0,  0} },
    { .pos={ 1, -1, -1}, .uv={1, 0}, .normal={ 0, -1,  0}, .tangent={ 1,  0,  0} },
    { .pos={-1, -1, -1}, .uv={0, 0}, .normal={ 0, -1,  0}, .tangent={ 1,  0,  0} },
    { .pos={ 1,  1,  1}, .uv={0, 1}, .normal={ 0,  1,  0}, .tangent={-1,  0,  0} },
    { .pos={-1,  1,  1}, .uv={1, 1}, .normal={ 0,  1,  0}, .tangent={-1,  0,  0} },
    { .pos={-1,  1, -1}, .uv={1, 0}, .normal={ 0,  1,  0}, .tangent={-1,  0,  0} },
    { .pos={ 1,  1, -1}, .uv={0, 0}, .normal={ 0,  1,  0}, .tangent={-1,  0,  0} },
});

constexpr auto indices = std::to_array<uint32_t>({
     2,  1,  0,  3,  2,  0,
     6,  5,  4,  7,  6,  4,
    10,  9,  8, 11, 10,  8,
    14, 13, 12, 15, 14, 12,
    18, 17, 16, 19, 18, 16,
    22, 21, 20, 23, 22, 20
});

AssetLoader<Mesh>::AssetLoader(gfx::Device* device, std::filesystem::path path)
    : m_device(device), m_source(std::move(path))
{
    assert(std::filesystem::is_regular_file(std::get<std::filesystem::path>(m_source)));
}

AssetLoader<Mesh>::AssetLoader(gfx::Device* device, BuiltInMesh builtInMesh)
    : m_device(device), m_source(builtInMesh)
{
}

std::shared_ptr<Mesh> AssetLoader<Mesh>::load(gfx::CommandBuffer& commandBuffer) const
{
    return std::visit([&](const auto& source) { return load(source, commandBuffer); }, m_source);
}

std::shared_ptr<Mesh> AssetLoader<Mesh>::load(const std::filesystem::path& meshPath, gfx::CommandBuffer& commandBuffer) const
{
    gltf::Expected<gltf::GltfDataBuffer> gltfDataBuffer = gltf::GltfDataBuffer::FromPath(meshPath);
    if (gltfDataBuffer.error() != gltf::Error::None)
        throw std::runtime_error(std::format("{}: failed to load glTF: data error", meshPath.string()));

    gltf::Parser parser;
    gltf::Expected<gltf::Asset> asset = parser.loadGltf(gltfDataBuffer.get(), meshPath.parent_path(), gltf::Options::LoadExternalBuffers | gltf::Options::GenerateMeshIndices, gltf::Category::Asset | gltf::Category::Buffers | gltf::Category::BufferViews | gltf::Category::Accessors | gltf::Category::Images | gltf::Category::Textures | gltf::Category::Materials | gltf::Category::Meshes | gltf::Category::Nodes | gltf::Category::Scenes);
    if (asset.error() != gltf::Error::None)
        throw std::runtime_error(std::format("{}: failed to load glTF: asset error", meshPath.string()));

    if (asset->scenes.empty())
        throw std::runtime_error(std::format("{}: failed to load glTF: no scene", meshPath.string()));
    const gltf::Scene& scene = asset->defaultScene ? asset->scenes[*asset->defaultScene] : asset->scenes.front();

    auto mesh = std::make_shared<Mesh>();

    auto addNode = [&](this auto&& self, std::vector<SubMesh>& dst, const gltf::Node& node, const glm::mat4x4& additionalTransform = glm::mat4x4(1.0f)) -> void
    {
        glm::mat4x4 transform;
        if (const gltf::math::fmat4x4* nodeTransform = std::get_if<gltf::math::fmat4x4>(&node.transform))
            transform = additionalTransform * glm::make_mat4x4(nodeTransform->data());
        else if (const gltf::TRS* nodeTRS = std::get_if<gltf::TRS>(&node.transform))
        {
            auto matrix = glm::mat4x4(1.0f);
            matrix = glm::translate(matrix, glm::make_vec3(nodeTRS->translation.data()));
            matrix = glm::rotate(matrix, nodeTRS->rotation.x(), glm::vec3(1, 0, 0));
            matrix = glm::rotate(matrix, nodeTRS->rotation.y(), glm::vec3(0, 1, 0));
            matrix = glm::rotate(matrix, nodeTRS->rotation.z(), glm::vec3(0, 0, 1));
            matrix = glm::scale(matrix, glm::make_vec3(nodeTRS->scale.data()));
            transform = additionalTransform * matrix;
        }
        else
            std::unreachable();

        std::size_t dstSize = dst.size();
        if (gltf::Optional<std::size_t> meshIndex = node.meshIndex)
        {
            for (uint32_t i = 0; const gltf::Primitive& primitive : asset->meshes[*meshIndex].primitives)
            {
                if (primitive.type != gltf::PrimitiveType::Triangles)
                    throw std::runtime_error(std::format("{}: failed to load glTF: unsuported primitive type", meshPath.string()));

                auto findAccessor = [&](std::string_view name) -> const gltf::Accessor* {
                    auto it = std::ranges::find_if(primitive.attributes, [&](const gltf::Attribute& attribute) -> bool { return attribute.name == name; });
                    return it == primitive.attributes.end() ? nullptr : &asset->accessors[it->accessorIndex];
                };

                const gltf::Accessor* posAccessor = findAccessor("POSITION");
                if (posAccessor == nullptr)
                    throw std::runtime_error(std::format("{}: failed to load glTF: pos accessor not found", meshPath.string()));

                const gltf::Accessor* indicesAccessor = primitive.indicesAccessor.has_value() ? &asset->accessors[*primitive.indicesAccessor] : nullptr;
                if (indicesAccessor == nullptr)
                    throw std::runtime_error(std::format("{}: failed to load glTF: index accessor not found", meshPath.string()));

                const gltf::Accessor* uvAccessor = findAccessor("TEXCOORD_0");
                const gltf::Accessor* normalAccessor = findAccessor("NORMAL");
                const gltf::Accessor* tangentAccessor = findAccessor("TANGENT");

                dst.emplace_back(SubMesh{
                    .name = std::format("{}{}", asset->meshes[*meshIndex].name, i++),
                    .transform = transform,
                    .vertexBuffer = newDeviceLocalBuffer(*m_device, commandBuffer, gfx::BufferUsage::vertexBuffer, std::views::iota(std::size_t(0), posAccessor->count) | std::views::transform([&](std::size_t i) -> Vertex {
                        return Vertex{
                            .pos = gltf::getAccessorElement<glm::vec3>(asset.get(), *posAccessor, i),
                            .uv = uvAccessor ? uvAccessor->count > i ? gltf::getAccessorElement<glm::vec2>(asset.get(), *uvAccessor, i) : glm::vec2{} : glm::vec2{},
                            .normal = normalAccessor ? normalAccessor->count > i ? gltf::getAccessorElement<glm::vec3>(asset.get(), *normalAccessor, i) : glm::vec3{} : glm::vec3{},
                            .tangent = tangentAccessor ? tangentAccessor->count > i ? gltf::getAccessorElement<glm::vec3>(asset.get(), *tangentAccessor, i) : glm::vec3{} : glm::vec3{}
                        };
                    })),
                    .indexBuffer = newDeviceLocalBuffer(*m_device, commandBuffer, gfx::BufferUsage::indexBuffer, std::views::iota(std::size_t(0), indicesAccessor->count) | std::views::transform([&](std::size_t i) -> uint32_t {
                        return gltf::getAccessorElement<uint32_t>(asset.get(), *indicesAccessor, i);
                    })),
                });
            }
        }

        for (const gltf::Node& childNode : node.children | std::views::transform([&](std::size_t i) -> gltf::Node { return asset->nodes[i]; }))
        {
            if (dstSize == dst.size())
                self(dst, childNode, transform);
            else
                self(dst, childNode);
        }
    };

    commandBuffer.beginBlitPass();
    for (const gltf::Node& node : scene.nodeIndices | std::views::transform([&](std::size_t i) -> gltf::Node { return asset->nodes[i]; }))
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
                .subMeshes = {}
            }}
        });
        commandBuffer.endBlitPass();
        return mesh;
        break;
    }
}

} // namespace GE
