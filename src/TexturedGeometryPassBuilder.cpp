#include "Game-Engine/FramePassBuilder.hpp"
#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/ECSView.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/ICamera.hpp"
#include "Game-Engine/Mesh.hpp"
#include "Game-Engine/Material.hpp"

#include "TextureTable.hpp"

#include "shaders/FrameData.slang"
#include "shaders/Light.slang"
#include "shaders/textured.slang"

#include <cstddef>
#include <glm/gtc/matrix_transform.hpp>

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#include <cassert>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <utility>
#include <vector>
#include <cstdint>

namespace GE
{

namespace
{

using Renderables = std::map<
    std::pair<std::shared_ptr<gfx::Buffer>, std::shared_ptr<gfx::Buffer>>, // vertex buffer / index buffer
    std::vector<std::pair<glm::mat4x4, uint32_t>> // model matrix / material index
>;

struct PushConstant
{
    uint32_t materialIndex;
    alignas(16) glm::mat4x4 modelMatrix;
};

static_assert(sizeof(PushConstant) == 80);

}

TexturedGeometryPassBuilder::TexturedGeometryPassBuilder(
    std::function<const Scene&()> sceneProvider,
    std::function<const ICamera*()> cameraOverrideProvider
)
    : m_sceneProvider(std::move(sceneProvider))
    , m_cameraOverrideProvider(std::move(cameraOverrideProvider))
{
    assert(m_sceneProvider);
    assert(m_cameraOverrideProvider);
}

FramePass TexturedGeometryPassBuilder::build() const
{
    ZoneScopedN("TexturedGeometryPassBuilder::build");

    GE::FramePass framePass = FramePassBuilderBase<TexturedGeometryPassBuilder>::build();
    const std::string colorAttachmentName = m_colorAttachment.texture;

    framePass.constantBufferDeclarations = {
        { .name = "frameData", .size = sizeof(shader::FrameData) },
    };
    framePass.structuredBufferDeclarations = {
        { .name = "directionalLights" },
        { .name = "pointLights" },
        { .name = "materials" }
    };
    framePass.usedBuffers.insert(framePass.usedBuffers.end(), {
        "frameData", "directionalLights", "pointLights", "materials"
    });

    std::shared_ptr<Renderables> renderables = std::make_shared<Renderables>();

    framePass.setup = [sceneProvider=m_sceneProvider, cameraProvider=m_cameraOverrideProvider, colorAttachmentName, renderables](FramePassSetupContext& ctx)
    {
        ZoneScopedN("TexturedGeometryPass::setup");

        const Scene& scene = sceneProvider();
        assert(colorAttachmentName.empty() == false);

        const std::shared_ptr<gfx::Texture>& colorAttachment = ctx.textureMap.at(colorAttachmentName);
        assert(colorAttachment->height() != 0);
        const float aspectRatio = static_cast<float>(colorAttachment->width()) / static_cast<float>(colorAttachment->height());

        shader::FrameData& frameData = *ctx.constantBuffers.at("frameData")->content<shader::FrameData>();
        if (const ICamera* camera = cameraProvider())
        {
            frameData.vpMatrix = camera->viewProjectionMatrix(aspectRatio);
            frameData.cameraPosition = camera->position();
        }
        else
        {
            const_Entity activeCamera = scene.activeCamera();
            assert(activeCamera.world != nullptr);
            assert(activeCamera.world->isValidEntityID(activeCamera.entityId));
            assert(activeCamera.has<TransformComponent>());
            assert(activeCamera.has<CameraComponent>());

            const glm::vec3 position = activeCamera.worldPosition();
            const glm::quat rotation = activeCamera.worldRotation();
            const glm::vec3 direction = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
            const glm::vec3 up = rotation * glm::vec3(0.0f, 1.0f, 0.0f);

            frameData.vpMatrix = activeCamera.get<CameraComponent>().projectionMatrix(aspectRatio) * glm::lookAt(position, position + direction, up);
            frameData.cameraPosition = position;
        }
        frameData.ambientLightColor = glm::vec3(1.0f, 1.0f, 1.0f) * 0.1f;

        TracyCZoneN(TexturedGeometryPassCollectLight, "TexturedGeometryPass::collect lights", true);

        std::vector<shader::DirectionalLight> directionalLights;
        std::vector<shader::PointLight> pointLights;
        for (GE::const_Entity entity : scene.ecsWorld() | const_ECSView<TransformComponent, LightComponent>() | MakeEntity())
        {
            const LightComponent& light = entity.get<LightComponent>();
            switch (light.type)
            {
            case LightComponent::Type::directional:
                directionalLights.push_back({
                    .position = entity.worldTransform()[3],
                    .color = light.color * light.intentsity,
                });
                break;
            case LightComponent::Type::point:
                pointLights.push_back({
                    .position = entity.worldTransform()[3],
                    .color = light.color * light.intentsity,
                    .attenuation = light.attenuation
                });
                break;
            }
        }

        const size_t directionalCount = directionalLights.size();
        const size_t pointCount = pointLights.size();

        assert(directionalCount <= static_cast<size_t>(std::numeric_limits<int>::max()));
        assert(pointCount <= static_cast<size_t>(std::numeric_limits<int>::max()));
        frameData.directionalLightCount = static_cast<int>(directionalCount);
        frameData.pointLightCount = static_cast<int>(pointCount);

        const size_t directionalBufferBytes = (directionalCount > 0 ? directionalCount : 1) * sizeof(shader::DirectionalLight);
        const size_t pointBufferBytes = (pointCount > 0 ? pointCount : 1) * sizeof(shader::PointLight);
        assert(directionalBufferBytes <= std::numeric_limits<uint32_t>::max());
        assert(pointBufferBytes <= std::numeric_limits<uint32_t>::max());

        ctx.setStructuredBufferContent("directionalLights", directionalLights.data(), static_cast<uint32_t>(directionalBufferBytes));
        ctx.setStructuredBufferContent("pointLights", pointLights.data(), static_cast<uint32_t>(pointBufferBytes));

        TracyCZoneEnd(TexturedGeometryPassCollectLight);

        TracyCZoneN(TexturedGeometryPassCollectRenderables, "TexturedGeometryPass::collect renderables", true);
        std::vector<shader::textured::Material> materials;
        std::map<std::shared_ptr<Material>, uint32_t> materialIndices;

        renderables->clear();
        for (auto entity : scene.ecsWorld() | const_ECSView<TransformComponent, MeshComponent>() | MakeEntity())
        {
            const MeshComponent& meshComponent = entity.get<MeshComponent>();
            if (!ctx.assetManager.isAssetLoaded(meshComponent))
                continue;

            std::shared_ptr<Mesh> loadedMesh = ctx.assetManager.getAsset<Mesh>(meshComponent.id);

            std::function<void(const SubMesh&, glm::mat4)> addSubmesh = [&](const SubMesh& submesh, const glm::mat4& transform) {
                TracyCZoneN(modelMatrixCalculationCtx, "addSubmesh::calculate model matrix", true);
                glm::mat4 modelMatrix = transform * submesh.transform;
                TracyCZoneEnd(modelMatrixCalculationCtx);

                for (auto& childSubmesh : submesh.subMeshes)
                    addSubmesh(childSubmesh, modelMatrix);

                TracyCZoneN(emplaceMaterial, "addSubmesh::emplace material", true);
                assert(submesh.material != nullptr);
                auto [it, inserted] = materialIndices.try_emplace(submesh.material, static_cast<uint32_t>(materials.size()));
                if (inserted) {
                    materials.push_back(shader::textured::Material{
                        .diffuseColor = submesh.material->diffuseColor,
                        .diffuseTextureIdx = ctx.textureTable.textureIndex(submesh.material->diffuseTexture),
                        .specularColor = submesh.material->specularColor,
                        .shininess = submesh.material->shininess
                    });
                }
                TracyCZoneEnd(emplaceMaterial);

                TracyCZoneN(appendMatrix, "addSubmesh::append model matrix", true);
                (*renderables)[std::make_pair(submesh.vertexBuffer, submesh.indexBuffer)].emplace_back(modelMatrix, it->second);
                TracyCZoneEnd(appendMatrix);
            };

            glm::mat4x4 worldTransform = entity.worldTransform();
            for (auto& submesh : loadedMesh->subMeshes)
                addSubmesh(submesh, worldTransform);
        }

        ctx.setStructuredBufferContent("materials", materials.data(), static_cast<uint32_t>(materials.size()) * sizeof(shader::textured::Material));
        TracyCZoneEnd(TexturedGeometryPassCollectRenderables);
    };

    framePass.execute = [sceneProvider=m_sceneProvider, renderables](FramePassExecuteContext& ctx)
    {
        ZoneScopedN("TexturedGeometryPass::execute");

        if (renderables->empty())
            return;

        std::shared_ptr<gfx::ParameterBlock> frameDataPBlock = ctx.parameterBlockPool.get(ctx.frameDataBlockLayout);
        frameDataPBlock->setBinding(0, ctx.bufferMap.at("frameData"));
        frameDataPBlock->setBinding(1, ctx.bufferMap.at("directionalLights"));
        frameDataPBlock->setBinding(2, ctx.bufferMap.at("pointLights"));

        std::shared_ptr<gfx::ParameterBlock> materialPBlock = ctx.parameterBlockPool.get(ctx.texturedMaterialPBlockLayout);
        materialPBlock->setBinding(0, ctx.bufferMap.at("materials"));

        ctx.commandBuffer.usePipeline(ctx.texturedPipeline);
        ctx.commandBuffer.setParameterBlock(ctx.textureTableBlock, 0);
        ctx.commandBuffer.setParameterBlock(frameDataPBlock, 1);
        ctx.commandBuffer.setParameterBlock(materialPBlock, 2);

        for (auto& [key, value] : *renderables) {
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
    };

    return framePass;
}

}
