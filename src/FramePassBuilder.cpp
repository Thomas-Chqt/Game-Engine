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
#include <memory>
#include <vector>
#include <future>

namespace GE
{

FlatGeometryPassBuilder::FlatGeometryPassBuilder(const Scene* scene, const ICamera* camera)
    : m_scene(scene)
    , m_camera(camera)
{
    assert(m_scene);
    assert(m_camera);
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

    framePass.setup = [scene=m_scene, camera=m_camera](FramePassSetupContext& ctx)
    {
        assert(scene);
        assert(camera);

        shader::FrameData& frameData = *ctx.constantBuffers.at("frameData")->content<shader::FrameData>();
        frameData.vpMatrix = camera->viewProjectionMatrix();
        frameData.cameraPosition = camera->position();
        frameData.ambientLightColor = glm::vec3(1.0f, 1.0f, 1.0f) * 0.1f;

        std::vector<shader::DirectionalLight> directionalLights;
        std::vector<shader::PointLight> pointLights;
        for (auto row : scene->ecsWorld() | const_ECSView<TransformComponent, LightComponent>())
        {
            const_Entity entity = static_cast<const_Entity>(row);
            const LightComponent& light = GE::get<1>(row);
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

        frameData.directionalLightCount = directionalLights.size();
        frameData.pointLightCount = pointLights.size();

        ctx.setStructuredBufferContent("directionalLights", directionalLights.data(), (directionalLights.size() > 0 ?  directionalLights.size() : 1) * sizeof(shader::DirectionalLight));
        ctx.setStructuredBufferContent("pointLights", pointLights.data(), (pointLights.size() > 0 ? pointLights.size() : 1 ) * sizeof(shader::PointLight));

        shader::flat_color::Material& material = *ctx.constantBuffers.at("material")->content<shader::flat_color::Material>();
        material.diffuseColor = glm::vec4(1.0f);
        material.specularColor = glm::vec4(0.0f);
        material.shininess = 0.0f;
    };

    framePass.execute = [scene=m_scene](FramePassExecuteContext& ctx)
    {
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

        for (auto row : scene->ecsWorld() | const_ECSView<TransformComponent, MeshComponent>())
        {
            const_Entity entity = static_cast<const_Entity>(row);
            const MeshComponent meshId = GE::get<1>(row);
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
        }
    };

    return framePass;
}

}
