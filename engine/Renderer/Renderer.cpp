/*
 * ---------------------------------------------------
 * Renderer.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/09 15:54:42
 * ---------------------------------------------------
 */

#include "Renderer/Renderer.hpp"
#include "ECS/Components.hpp"
#include "ECS/ECSView.hpp"
#include "Graphics/Enums.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "Graphics/GraphicPipeline.hpp"
#include "Math/Matrix.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/Types.hpp"
#include <cassert>

namespace GE
{

static utils::SharedPtr<gfx::GraphicPipeline> makeGraphicPipeline(gfx::GraphicAPI& api)
{
    gfx::Shader::Descriptor shaderDescriptor;

    shaderDescriptor.type = gfx::ShaderType::vertex;
    #ifdef GFX_BUILD_METAL
        shaderDescriptor.mtlShaderLibPath = MTL_SHADER_LIB;
        shaderDescriptor.mtlFunction = "default_vs";
    #endif
    #ifdef GFX_BUILD_OPENGL
        shaderDescriptor.openglCode = utils::String::contentOfFile(OPENGL_SHADER_DIR"/default.vs");
    #endif

    utils::SharedPtr<gfx::Shader> vs = api.newShader(shaderDescriptor);

    shaderDescriptor.type = gfx::ShaderType::fragment;
    #ifdef GFX_BUILD_METAL
        shaderDescriptor.mtlShaderLibPath = MTL_SHADER_LIB;
        shaderDescriptor.mtlFunction = "default_fs";
    #endif
    #ifdef GFX_BUILD_OPENGL
        shaderDescriptor.openglCode = utils::String::contentOfFile(OPENGL_SHADER_DIR"/default.fs");
    #endif

    utils::SharedPtr<gfx::Shader> fs = api.newShader(shaderDescriptor);

    gfx::VertexLayout vertexLayout;
    vertexLayout.attributes.append({gfx::VertexAttributeFormat::vec3f, offsetof(Renderer::Vertex, pos)});
    vertexLayout.attributes.append({gfx::VertexAttributeFormat::vec2f, offsetof(Renderer::Vertex, uv)});
    vertexLayout.attributes.append({gfx::VertexAttributeFormat::vec3f, offsetof(Renderer::Vertex, normal)});
    vertexLayout.stride = sizeof(Renderer::Vertex);

    gfx::GraphicPipeline::Descriptor gfxPipelineDesc;
    gfxPipelineDesc.vertexLayout = vertexLayout;
    gfxPipelineDesc.vertexShader = vs;
    gfxPipelineDesc.fragmentShader = fs;
    return api.newGraphicsPipeline(gfxPipelineDesc);
}

Renderer::Renderer(const utils::SharedPtr<gfx::GraphicAPI>& api)
    : m_graphicAPI(api)
{
    m_graphicAPI->initImgui();

    m_gfxPipeline = makeGraphicPipeline(*m_graphicAPI);

    m_vpMatrix.alloc(*m_graphicAPI);
    m_lightsBuffer.alloc(*m_graphicAPI);
}

void Renderer::beginScene(const Renderer::Camera& cam, const utils::SharedPtr<gfx::RenderTarget>& target)
{
    m_renderTarget = target;

    math::mat4x4 projectionMatrix = cam.projectionMatrix;
    projectionMatrix[0][0] /= (float)target->width() / (float)target->height();

    m_vpMatrix.map();
    m_vpMatrix.content() = projectionMatrix * cam.viewMatrix;
    m_vpMatrix.unmap();

    m_lightsBuffer.map();
    m_lightsBuffer.content().pointLightCount = 0;

    m_renderables.clear();
}

void Renderer::addRenderable(const Renderer::Renderable& renderable)
{
    m_renderables.append(renderable);
}

void Renderer::addRenderables(const ECSWorld& world, const AssetManager& assetManager)
{
    const_ECSView<TransformComponent, MeshComponent>(world).onEach([&](const Entity entity, const TransformComponent& transform, const MeshComponent& mesh) {
        if (mesh.assetId.is_nil() == false)
        {
            assert(assetManager.loadedMeshes().contain(mesh.assetId));
            math::mat4x4 entityWorldTransform = entity.worldTransform();
            for (auto& subMesh : assetManager.loadedMeshes()[mesh.assetId].subMeshes)
            {
                addRenderable(Renderer::Renderable{
                    subMesh.vertexBuffer, subMesh.indexBuffer,
                    entityWorldTransform * subMesh.transform,
                });
            }
        }
    });
}

void Renderer::addPointLight(const Renderer::PointLight& pointLight)
{
    assert(m_lightsBuffer.content().pointLightCount < 32);
    utils::uint32 idx = m_lightsBuffer.content().pointLightCount++;
    m_lightsBuffer.content().pointLights[idx] = pointLight;
}

void Renderer::addLights(const ECSWorld& world)
{
    const_ECSView<TransformComponent, LightComponent>(world).onEach([&](const Entity, const TransformComponent& transform, const LightComponent& light) {
        switch (light.type)
        {
        case LightComponent::Type::point:
            addPointLight({transform.position, light.color, light.intentsity});
            break;
        default:
            UNREACHABLE
        }
    });
}

void Renderer::endScene()
{
    m_lightsBuffer.unmap();
}

void Renderer::render()
{
    m_graphicAPI->beginFrame();
    {
        m_graphicAPI->setLoadAction(gfx::LoadAction::clear);
        m_graphicAPI->beginRenderPass(m_renderTarget);
        {
            m_graphicAPI->useGraphicsPipeline(m_gfxPipeline);

            m_graphicAPI->setVertexBuffer(m_vpMatrix.buffer(), m_gfxPipeline->getVertexBufferIndex("vpMatrixBuffer"));
            m_graphicAPI->setFragmentBuffer(m_lightsBuffer.buffer(), m_gfxPipeline->getFragmentBufferIndex("lightsBuffer"));

            for (auto& renderable : m_renderables)
            {
                m_graphicAPI->setVertexUniform(renderable.modelMatrix, m_gfxPipeline->getVertexUniformIndex("modelMatrix"));
                m_graphicAPI->setVertexBuffer(renderable.vertexBuffer, 0);
                m_graphicAPI->drawIndexedVertices(renderable.indexBuffer);
            }
        }
        m_graphicAPI->endRenderPass();

        if (m_onImGuiRender)
        {
            m_graphicAPI->setLoadAction(gfx::LoadAction::load);
            m_graphicAPI->beginImguiRenderPass();
            {
                m_onImGuiRender();
            }
            m_graphicAPI->endRenderPass();
        }
    }
    m_graphicAPI->endFrame();
}

}