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

#include <future>
#include <glm/gtc/matrix_transform.hpp>

#include <cassert>
#include <memory>

namespace GE
{

FramePass FlatGeometryPassBuilder::build() const
{
    GE::FramePass framePass;

    framePass.colorAttachments = { GE::ColorAttachmentUsage{
        .name = m_colorAttachmentName,
        .loadAction = gfx::LoadAction::clear,
        .clearColor = { m_clearColor.x, m_clearColor.y, m_clearColor.z, 1.0f },
    }};

    if (m_depthAttachmentName.has_value())
    {
        framePass.depthAttachment = GE::DepthAttachmentUsage{
            .name = *m_depthAttachmentName,
            .loadAction = gfx::LoadAction::clear,
            .clearDepth = m_clearDepth,
        };
    }

    framePass.setup = [scene=m_scene](FramePassSetupContext& ctx)
    {
        assert(scene);

        auto& cameraTransform = scene->activeCamera().get<TransformComponent>();

        auto rotationMat = glm::mat4x4(1.0f);
        rotationMat = glm::rotate(rotationMat, cameraTransform.rotation.y, glm::vec3(0, 1, 0));
        rotationMat = glm::rotate(rotationMat, cameraTransform.rotation.x, glm::vec3(1, 0, 0));
        rotationMat = glm::rotate(rotationMat, cameraTransform.rotation.z, glm::vec3(0, 0, 1));

        shader::FrameData& frameData = *ctx.bufferMap["frameData"]->content<shader::FrameData>();
        shader::DirectionalLight* directionalLights = ctx.bufferMap["directionalLights"]->content<shader::DirectionalLight>();
        shader::PointLight* pointLights = ctx.bufferMap["pointLights"]->content<shader::PointLight>();
        shader::flat_color::Material& material = *ctx.bufferMap["material"]->content<shader::flat_color::Material>();

        glm::vec3 pos = cameraTransform.position;
        glm::vec3 dir = rotationMat * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
        glm::vec3 up = rotationMat * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

        frameData.cameraPosition = cameraTransform.position;
        frameData.ambientLightColor = glm::vec3(1.0f, 1.0f, 1.0f) * 0.1f;

        frameData.directionalLightCount = 0;
        frameData.pointLightCount = 0;
        const_ECSView<TransformComponent, LightComponent>(&scene->ecsWorld()).onEach([&](const_Entity, const TransformComponent& transform, const LightComponent& light)
        {
            switch (light.type)
            {
            case LightComponent::Type::directional:
                directionalLights[frameData.directionalLightCount++] = {
                    .position = transform.position,
                    .color = light.color * light.intentsity,
                };
            case LightComponent::Type::point:
                pointLights[frameData.pointLightCount++] = {
                    .position = transform.position,
                    .color = light.color * light.intentsity,
                    .attenuation = light.attenuation
                };
            }
        });

        material.diffuseColor = glm::vec4(1.0f);
        material.specularColor = glm::vec4(0.0f);
        material.shininess = 0.0f;
    };

    framePass.execute = [scene=m_scene](FramePassContext& ctx)
    {
        assert(scene);

        auto& cameraTransform = scene->activeCamera().get<TransformComponent>();

        auto rotationMat = glm::mat4x4(1.0f);
        rotationMat = glm::rotate(rotationMat, cameraTransform.rotation.y, glm::vec3(0, 1, 0));
        rotationMat = glm::rotate(rotationMat, cameraTransform.rotation.x, glm::vec3(1, 0, 0));
        rotationMat = glm::rotate(rotationMat, cameraTransform.rotation.z, glm::vec3(0, 0, 1));

        glm::vec3 pos = cameraTransform.position;
        glm::vec3 dir = rotationMat * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
        glm::vec3 up = rotationMat * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

        shader::FrameData& frameData = *ctx.bufferMap["frameData"]->content<shader::FrameData>();

        const float aspectRatio = static_cast<float>(ctx.renderSize.first) / static_cast<float>(ctx.renderSize.second == 0 ? 1 : ctx.renderSize.second);
        frameData.vpMatrix = scene->activeCamera().get<CameraComponent>().projectionMatrix(aspectRatio) * glm::lookAt(pos, pos + dir, up);

        ctx.commandBuffer.usePipeline(ctx.gfxPipeline);

        std::shared_ptr<gfx::ParameterBlock> frameDataPBlock = ctx.parameterBlockPool.get(ctx.frameDataBlockLayout);
        frameDataPBlock->setBinding(0, ctx.bufferMap["frameData"]);
        frameDataPBlock->setBinding(1, ctx.bufferMap["directionalLights"]);
        frameDataPBlock->setBinding(2, ctx.bufferMap["pointLights"]);

        std::shared_ptr<gfx::ParameterBlock> materialPBlock = ctx.parameterBlockPool.get(ctx.materialBlockLayout);
        materialPBlock->setBinding(0, ctx.bufferMap["material"]);

        AssetManager* assetManager;
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
