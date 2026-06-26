/*
 * ---------------------------------------------------
 * BuiltInPasses.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/BuiltInPasses.hpp"

#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/ECSView.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Material.hpp"
#include "Game-Engine/Mesh.hpp"

#include "shaders/FrameData.slang"
#include "shaders/Light.slang"
#include "shaders/textured.slang"

#include <Graphics/Buffer.hpp>
#include <Graphics/ParameterBlock.hpp>

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <glm/glm.hpp>
#include <imgui.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

namespace GE
{

namespace
{

using Renderables = std::map<
    std::pair<std::shared_ptr<gfx::Buffer>, std::shared_ptr<gfx::Buffer>>,
    std::vector<std::pair<glm::mat4x4, uint32_t>>
>;

struct PushConstant
{
    uint32_t materialIndex;
    alignas(16) glm::mat4x4 modelMatrix;
};

static_assert(sizeof(PushConstant) == 80);

}

TexturedGeometryPass::TexturedGeometryPass(const Scene& scene, glm::mat4 viewProjectionMatrix, glm::vec3 cameraPosition)
    : m_scene(&scene)
    , m_viewProjectionMatrix(viewProjectionMatrix)
    , m_cameraPosition(cameraPosition)
{
}

void TexturedGeometryPass::record(FrameGraphBuilder& builder) const
{
    ZoneScopedN("TexturedGeometryPass::record");

    assert(m_scene);
    const Scene& scene = *m_scene;
    AssetManager& assetManager = builder.assetManager();

    uint32_t directionalLightCount = 0;
    uint32_t pointLightCount = 0;
    for (const_Entity entity : scene.ecsWorld() | const_ECSView<TransformComponent, LightComponent>() | MakeEntity())
    {
        switch (entity.get<LightComponent>().type)
        {
        case LightComponent::Type::directional:
            ++directionalLightCount;
            break;
        case LightComponent::Type::point:
            ++pointLightCount;
            break;
        }
    }

    FrameGraph::BufferRef frameDataBuffer = builder.newConstantBuffer<shader::FrameData>();
    shader::FrameData& frameData = builder.constantBufferContent<shader::FrameData>(frameDataBuffer);
    frameData.vpMatrix = m_viewProjectionMatrix;
    frameData.cameraPosition = m_cameraPosition;
    frameData.ambientLightColor = glm::vec3(1.0f, 1.0f, 1.0f) * 0.1f;
    frameData.directionalLightCount = static_cast<int>(directionalLightCount);
    frameData.pointLightCount = static_cast<int>(pointLightCount);

    FrameGraph::BufferRef directionalLightsBuffer = builder.newStructuredBuffer<shader::DirectionalLight>(std::max<size_t>(directionalLightCount, 1));
    FrameGraph::BufferRef pointLightsBuffer = builder.newStructuredBuffer<shader::PointLight>(std::max<size_t>(pointLightCount, 1));
    std::span<shader::DirectionalLight> directionalLights = builder.structuredBufferContent<shader::DirectionalLight>(directionalLightsBuffer);
    std::span<shader::PointLight> pointLights = builder.structuredBufferContent<shader::PointLight>(pointLightsBuffer);

    size_t directionalIdx = 0;
    size_t pointIdx = 0;
    for (const_Entity entity : scene.ecsWorld() | const_ECSView<TransformComponent, LightComponent>() | MakeEntity())
    {
        const LightComponent& light = entity.get<LightComponent>();
        switch (light.type)
        {
        case LightComponent::Type::directional:
            directionalLights[directionalIdx++] = {
                .position = entity.worldTransform()[3],
                .color = light.color * light.intentsity,
            };
            break;
        case LightComponent::Type::point:
            pointLights[pointIdx++] = {
                .position = entity.worldTransform()[3],
                .color = light.color * light.intentsity,
                .attenuation = light.attenuation
            };
            break;
        }
    }

    TracyCZoneN(TexturedGeometryPassRecordRenderables, "TexturedGeometryPass::record renderables", true);
    Renderables renderables;
    std::vector<shader::textured::Material> materials;
    std::map<std::shared_ptr<Material>, uint32_t> materialIndices;
    for (const_Entity entity : scene.ecsWorld() | const_ECSView<TransformComponent, MeshComponent>() | MakeEntity())
    {
        const MeshComponent& meshComponent = entity.get<MeshComponent>();
        if (!assetManager.isAssetLoaded(meshComponent))
            continue;

        std::shared_ptr<Mesh> loadedMesh = assetManager.getAsset<Mesh>(meshComponent.id);

        std::function<void(const SubMesh&, glm::mat4)> addSubmesh = [&](const SubMesh& submesh, const glm::mat4& transform) {
            glm::mat4 modelMatrix = transform * submesh.transform;

            for (auto& childSubmesh : submesh.subMeshes)
                addSubmesh(childSubmesh, modelMatrix);

            assert(submesh.material != nullptr);
            auto [it, inserted] = materialIndices.try_emplace(submesh.material, static_cast<uint32_t>(materials.size()));
            if (inserted) {
                materials.push_back(shader::textured::Material{
                    .diffuseColor = submesh.material->diffuseColor,
                    .diffuseTextureIdx = builder.textureIndex(submesh.material->diffuseTexture),
                    .specularColor = submesh.material->specularColor,
                    .emissiveColor = submesh.material->emissiveColor,
                    .shininess = submesh.material->shininess
                });
            }

            renderables[std::make_pair(submesh.vertexBuffer, submesh.indexBuffer)].emplace_back(modelMatrix, it->second);
        };

        glm::mat4x4 worldTransform = entity.worldTransform();
        for (auto& submesh : loadedMesh->subMeshes)
            addSubmesh(submesh, worldTransform);
    }

    FrameGraph::BufferRef materialsBuffer = builder.newStructuredBuffer<shader::textured::Material>(std::max<size_t>(materials.size(), 1));
    std::ranges::copy(materials, builder.structuredBufferContent<shader::textured::Material>(materialsBuffer).begin());
    TracyCZoneEnd(TexturedGeometryPassRecordRenderables);

    const FrameGraph::TextureRef colorTexture = builder.texture("backBuffer");
    const FrameGraph::TextureRef depthTexture = builder.texture("depthBuffer");

    FramePass texturedGeometryPass = {
        .colorAttachments = {
            FrameGraph::Attachment{
                .texture = colorTexture,
                .loadAction = gfx::LoadAction::clear,
                .clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }
            }
        },
        .depthAttachment = FrameGraph::Attachment{
            .texture = depthTexture,
            .loadAction = gfx::LoadAction::clear,
            .clearDepth = 1.0f
        },
        .execute = [frameDataBuffer, directionalLightsBuffer, pointLightsBuffer, materialsBuffer, renderables=std::move(renderables)](FramePass::ExecuteContext& ctx){
            ZoneScopedN("TexturedGeometryPass::execute");

            if (renderables.empty())
                return;

            std::shared_ptr<gfx::ParameterBlock> frameDataPBlock = ctx.parameterBlockPool.get(ctx.frameDataBlockLayout);
            frameDataPBlock->setBinding(0, ctx.buffer(frameDataBuffer));
            frameDataPBlock->setBinding(1, ctx.buffer(directionalLightsBuffer));
            frameDataPBlock->setBinding(2, ctx.buffer(pointLightsBuffer));

            std::shared_ptr<gfx::ParameterBlock> materialPBlock = ctx.parameterBlockPool.get(ctx.texturedMaterialPBlockLayout);
            materialPBlock->setBinding(0, ctx.buffer(materialsBuffer));

            ctx.commandBuffer.usePipeline(ctx.texturedPipeline);
            ctx.commandBuffer.setParameterBlock(ctx.textureTableBlock, 0);
            ctx.commandBuffer.setParameterBlock(frameDataPBlock, 1);
            ctx.commandBuffer.setParameterBlock(materialPBlock, 2);

            for (auto& [key, value] : renderables) {
                auto& [vertexBuffer, indexBuffer] = key;

                ctx.commandBuffer.useVertexBuffer(vertexBuffer);
                for (auto& [modelMatrix, materialIdx] : value) {
                    PushConstant pc {
                        .materialIndex = materialIdx,
                        .modelMatrix = modelMatrix,
                    };

                    ctx.commandBuffer.setPushConstants(&pc);
                    ctx.commandBuffer.drawIndexedVertices(indexBuffer);
                }
            }
        }
    };

    builder.addPass(std::move(texturedGeometryPass));
}

void ImGuiPass::record(FrameGraphBuilder& builder) const
{
    ZoneScopedN("ImGuiPass::record");

    const FrameGraph::TextureRef targetTexture = builder.texture("windowBackBuffer");
    std::map<std::string, FrameGraph::TextureRef, std::less<>> textureRefs;
    std::vector<FrameGraph::TextureRef> sampledTextures;

    ImDrawData* drawData = ImGui::GetDrawData();
    assert(drawData);
    for (int i = 0; i < drawData->CmdListsCount; ++i) {
        for (ImDrawCmd& cmd : drawData->CmdLists[i]->CmdBuffer) {
            if (cmd.TexRef._TexID == 0)
                continue;
            assert(cmd.TexRef._TexData == nullptr);
            auto visitor = [&](auto& v) {
                if constexpr (std::is_same_v<std::remove_cvref_t<decltype(v)>, std::string>) {
                    FrameGraph::TextureRef textureRef = builder.texture(v);
                    textureRefs.try_emplace(v, textureRef);
                    if (std::ranges::find(sampledTextures, textureRef) == sampledTextures.end())
                        sampledTextures.push_back(textureRef);
                }
                else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(v)>, uint64_t>) {
                }
                else
                    std::unreachable();
            };
            std::visit(std::move(visitor), *std::bit_cast<std::variant<std::string, uint64_t>*>(cmd.TexRef._TexID));
        }
    }

    FramePass imguiPass = {
        .colorAttachments = {
            FrameGraph::Attachment{
                .texture = targetTexture,
                .loadAction = gfx::LoadAction::clear,
                .clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }
            }
        },
        .sampledTextures = std::move(sampledTextures),
        .execute = [textureRefs=std::move(textureRefs)](FramePass::ExecuteContext& ctx){
            ZoneScopedN("ImguiPass::execute");

            ImDrawData* drawData = ImGui::GetDrawData();
            for (int i = 0; i < drawData->CmdListsCount; ++i) {
                for (ImDrawCmd& cmd : drawData->CmdLists[i]->CmdBuffer) {
                    if (cmd.TexRef._TexID == 0)
                        continue;
                    assert(cmd.TexRef._TexData == nullptr);
                    auto vistor = [&](auto& v) {
                        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(v)>, std::string>) {
                            auto it = textureRefs.find(v);
                            assert(it != textureRefs.end());
                            std::shared_ptr<gfx::Texture> texture = ctx.texture(it->second);
                            if (texture->imTextureId().has_value() == false)
                                texture->initImTextureId();
                            cmd.TexRef._TexID = *texture->imTextureId();
                        }
                        else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(v)>, uint64_t>)
                            cmd.TexRef._TexID = v;
                        else
                            std::unreachable();
                    };
                    std::visit(std::move(vistor), *std::bit_cast<std::variant<std::string, uint64_t>*>(cmd.TexRef._TexID));
                }
            }
            ctx.commandBuffer.imGuiRenderDrawData(drawData);
        }
    };

    builder.addPass(std::move(imguiPass));
}

} // namespace GE
