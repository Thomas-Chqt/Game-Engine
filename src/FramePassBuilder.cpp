/*
 * ---------------------------------------------------
 * FramePassBuilder.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/FramePassBuilder.hpp"
#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/ECSView.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Mesh.hpp"

#include "shaders/FrameData.slang"
#include "shaders/Light.slang"
#include "shaders/flat_color.slang"

#include <glm/gtc/matrix_transform.hpp>

#include <cassert>
#include <memory>
#include <vector>
#include <future>

namespace GE
{

FlatGeometryPassBuilder::FlatGeometryPassBuilder(const Scene* scene, AssetManager* assetManager)
    : m_scene(scene), m_assetManager(assetManager)
{
}

FramePass FlatGeometryPassBuilder::build() const
{
    GE::FramePass framePass = FramePassBuilderBase<FlatGeometryPassBuilder>::build();

    framePass.constantBufferDeclarations = {
        { .name = "frameData", .size = sizeof(shader::FrameData) },
        { .name = "material",  .size = sizeof(shader::flat_color::Material) },
    };
    framePass.structuredBufferDeclarations = {
        { .name = "directionalLights" },
        { .name = "pointLights" },
    };
    framePass.usedBuffers.insert(framePass.usedBuffers.end(), {
        "frameData", "material", "directionalLights", "pointLights"
    });

    framePass.setup = [scene=m_scene, colorTexture=m_colorAttachment.texture](FramePassSetupContext& ctx)
    {
        assert(scene);

        auto& cameraTransform = scene->activeCamera().get<TransformComponent>();

        auto rotationMat = glm::mat4x4(1.0f);
        rotationMat = glm::rotate(rotationMat, cameraTransform.rotation.y, glm::vec3(0, 1, 0));
        rotationMat = glm::rotate(rotationMat, cameraTransform.rotation.x, glm::vec3(1, 0, 0));
        rotationMat = glm::rotate(rotationMat, cameraTransform.rotation.z, glm::vec3(0, 0, 1));

        shader::FrameData& frameData = *ctx.constantBuffers.at("frameData")->content<shader::FrameData>();

        glm::vec3 pos = cameraTransform.position;
        glm::vec3 dir = rotationMat * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
        glm::vec3 up = rotationMat * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

        auto& tex = ctx.textureMap.at(colorTexture);
        uint32_t texWidth = tex->width();
        uint32_t texHeight = tex->height();
        const float aspectRatio = static_cast<float>(texWidth) / static_cast<float>(texHeight == 0 ? 1 : texHeight);
        frameData.vpMatrix = scene->activeCamera().get<CameraComponent>().projectionMatrix(aspectRatio) * glm::lookAt(pos, pos + dir, up);

        frameData.cameraPosition = cameraTransform.position;
        frameData.ambientLightColor = glm::vec3(1.0f, 1.0f, 1.0f) * 0.1f;

        std::vector<shader::DirectionalLight> directionalLights;
        std::vector<shader::PointLight> pointLights;
        const_ECSView<TransformComponent, LightComponent>(&scene->ecsWorld()).onEach([&](const_Entity, const TransformComponent& transform, const LightComponent& light)
        {
            switch (light.type)
            {
            case LightComponent::Type::directional:
                directionalLights.push_back({
                    .position = transform.position,
                    .color = light.color * light.intentsity,
                });
                break;
            case LightComponent::Type::point:
                pointLights.push_back({
                    .position = transform.position,
                    .color = light.color * light.intentsity,
                    .attenuation = light.attenuation
                });
                break;
            }
        });

        frameData.directionalLightCount = directionalLights.size();
        frameData.pointLightCount = pointLights.size();

        ctx.setStructuredBufferContent("directionalLights", directionalLights.data(), directionalLights.size() * sizeof(shader::DirectionalLight));
        ctx.setStructuredBufferContent("pointLights", pointLights.data(), pointLights.size() * sizeof(shader::PointLight));

        shader::flat_color::Material& material = *ctx.constantBuffers.at("material")->content<shader::flat_color::Material>();
        material.diffuseColor = glm::vec4(1.0f);
        material.specularColor = glm::vec4(0.0f);
        material.shininess = 0.0f;
    };

    framePass.execute = [scene=m_scene, assetManager=m_assetManager](FramePassExecuteContext& ctx)
    {
        assert(scene);

        std::shared_ptr<gfx::ParameterBlock> frameDataPBlock = ctx.parameterBlockPool.get(ctx.frameDataBlockLayout);
        frameDataPBlock->setBinding(0, ctx.bufferMap.at("frameData"));
        if (ctx.bufferMap.contains("directionalLights"))
            frameDataPBlock->setBinding(1, ctx.bufferMap.at("directionalLights"));
        if (ctx.bufferMap.contains("pointLights"))
            frameDataPBlock->setBinding(2, ctx.bufferMap.at("pointLights"));

        std::shared_ptr<gfx::ParameterBlock> materialPBlock = ctx.parameterBlockPool.get(ctx.materialBlockLayout);
        materialPBlock->setBinding(0, ctx.bufferMap.at("material"));

        ctx.commandBuffer.usePipeline(ctx.gfxPipeline);
        ctx.commandBuffer.setParameterBlock(frameDataPBlock, 0);
        ctx.commandBuffer.setParameterBlock(materialPBlock, 1);

        const_ECSView<TransformComponent, MeshComponent>(&scene->ecsWorld()).onEach([&](const_Entity, const TransformComponent& transform, const MeshComponent meshId)
        {
            std::shared_future<const std::shared_ptr<Mesh>&> meshFuture = assetManager->loadAsset<Mesh>(meshId.id);
            if (meshFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                std::shared_ptr<Mesh> mesh = meshFuture.get();

                std::function<void(const SubMesh&, glm::mat4)> drawSubmesh = [&](const SubMesh& submesh, const glm::mat4& transform) {
                    glm::mat4 modelMatrix = transform * submesh.transform;

                    for (auto& childSubmesh : submesh.subMeshes)
                        drawSubmesh(childSubmesh, modelMatrix);

                    ctx.commandBuffer.setPushConstants(&modelMatrix);
                    ctx.commandBuffer.useVertexBuffer(submesh.vertexBuffer);
                    ctx.commandBuffer.drawIndexedVertices(submesh.indexBuffer);
                };

                for (auto& submesh : mesh->subMeshes)
                    drawSubmesh(submesh, transform);
            }
        });
    };

    return framePass;
}

}
