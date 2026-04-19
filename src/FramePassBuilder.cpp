/*
 * ---------------------------------------------------
 * FramePassBuilder.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/FramePassBuilder.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/ECSView.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/ICamera.hpp"
#include "Game-Engine/Mesh.hpp"

#include "shaders/FrameData.slang"
#include "shaders/Light.slang"
#include "shaders/flat_color.slang"

#include <glm/gtc/matrix_transform.hpp>

#include <cassert>
#include <functional>
#include <memory>
#include <vector>
#include <future>

namespace GE
{

FlatGeometryPassBuilder::FlatGeometryPassBuilder(const Scene* scene, const ICamera* camera)
    : FlatGeometryPassBuilder(
        [scene]() { return scene; },
        [camera]() { return camera; })
{
}

FlatGeometryPassBuilder::FlatGeometryPassBuilder(std::function<const Scene*()> sceneProvider, std::function<const ICamera*()> cameraProvider)
    : m_sceneProvider(std::move(sceneProvider))
    , m_cameraProvider(std::move(cameraProvider))
{
    assert(m_sceneProvider);
    assert(m_cameraProvider);
}

FramePass FlatGeometryPassBuilder::build() const
{
    GE::FramePass framePass = FramePassBuilderBase<FlatGeometryPassBuilder>::build();
    const std::string colorAttachmentName = m_colorAttachment.texture;

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

    framePass.setup = [sceneProvider=m_sceneProvider, cameraProvider=m_cameraProvider, colorAttachmentName](FramePassSetupContext& ctx)
    {
        const Scene* scene = sceneProvider();
        assert(scene);
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
            const_Entity activeCamera = scene->activeCamera();
            assert(activeCamera.world != nullptr);
            assert(activeCamera.world->isValidEntityID(activeCamera.entityId));
            assert(activeCamera.has<TransformComponent>());
            assert(activeCamera.has<CameraComponent>());

            const auto& transform = activeCamera.get<TransformComponent>();
            auto rotationMat = glm::mat4x4(1.0f);
            rotationMat = glm::rotate(rotationMat, transform.rotation.y, glm::vec3(0, 1, 0));
            rotationMat = glm::rotate(rotationMat, transform.rotation.x, glm::vec3(1, 0, 0));
            rotationMat = glm::rotate(rotationMat, transform.rotation.z, glm::vec3(0, 0, 1));

            const glm::vec3 position = activeCamera.worldTransform()[3];
            const glm::vec3 direction = rotationMat * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
            const glm::vec3 up = rotationMat * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

            frameData.vpMatrix = activeCamera.get<CameraComponent>().projectionMatrix(aspectRatio) * glm::lookAt(position, position + direction, up);
            frameData.cameraPosition = position;
        }
        frameData.ambientLightColor = glm::vec3(1.0f, 1.0f, 1.0f) * 0.1f;

        std::vector<shader::DirectionalLight> directionalLights;
        std::vector<shader::PointLight> pointLights;
        const_ECSView<TransformComponent, LightComponent>(&scene->ecsWorld()).onEach([&](const_Entity entity, const TransformComponent&, const LightComponent& light)
        {
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
        });

        frameData.directionalLightCount = directionalLights.size();
        frameData.pointLightCount = pointLights.size();

        ctx.setStructuredBufferContent("directionalLights", directionalLights.data(), (directionalLights.size() > 0 ?  directionalLights.size() : 1) * sizeof(shader::DirectionalLight));
        ctx.setStructuredBufferContent("pointLights", pointLights.data(), (pointLights.size() > 0 ? pointLights.size() : 1 ) * sizeof(shader::PointLight));

        shader::flat_color::Material& material = *ctx.constantBuffers.at("material")->content<shader::flat_color::Material>();
        material.diffuseColor = glm::vec4(1.0f);
        material.specularColor = glm::vec4(0.0f);
        material.shininess = 0.0f;
    };

    framePass.execute = [sceneProvider=m_sceneProvider](FramePassExecuteContext& ctx)
    {
        const Scene* scene = sceneProvider();
        assert(scene);

        std::shared_ptr<gfx::ParameterBlock> frameDataPBlock = ctx.parameterBlockPool.get(ctx.frameDataBlockLayout);
        frameDataPBlock->setBinding(0, ctx.bufferMap.at("frameData"));
        frameDataPBlock->setBinding(1, ctx.bufferMap.at("directionalLights"));
        frameDataPBlock->setBinding(2, ctx.bufferMap.at("pointLights"));

        std::shared_ptr<gfx::ParameterBlock> materialPBlock = ctx.parameterBlockPool.get(ctx.materialBlockLayout);
        materialPBlock->setBinding(0, ctx.bufferMap.at("material"));

        ctx.commandBuffer.usePipeline(ctx.gfxPipeline);
        ctx.commandBuffer.setParameterBlock(frameDataPBlock, 0);
        ctx.commandBuffer.setParameterBlock(materialPBlock, 1);

        const_ECSView<TransformComponent, MeshComponent>(&scene->ecsWorld()).onEach([&](const_Entity entity, const TransformComponent&, const MeshComponent meshId)
        {
            // ? maybe i should not load asset here, just skip them, so user is require to load assets befor using
            // ? loading here could cause unexpected asset load
            std::shared_future<const std::shared_ptr<Mesh>&> meshFuture = scene->assetManagerView().loadAsset<Mesh>(meshId);
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
                    drawSubmesh(submesh, entity.worldTransform());
            }
        });
    };

    return framePass;
}

}
