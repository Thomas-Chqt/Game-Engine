/*
 * ---------------------------------------------------
 * Renderer.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/09 15:54:42
 * ---------------------------------------------------
 */

#include "Renderer/Renderer.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "Graphics/GraphicPipeline.hpp"
#include "Graphics/Platform.hpp"
#include "Graphics/Window.hpp"
#include "Math/Matrix.hpp"
#include "Renderer/GPURessourceManager.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/Types.hpp"

namespace GE
{

utils::SharedPtr<gfx::GraphicPipeline> makeGraphicPipeline(const gfx::GraphicAPI& api)
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

void Renderer::setWindow(const utils::SharedPtr<gfx::Window>& window)
{
    m_window = window;
    m_graphicAPI = gfx::Platform::shared().newGraphicAPI(m_window);

    GPURessourceManager::shared().setGraphicAPI(m_graphicAPI);

    m_vpMatrix.alloc(*m_graphicAPI);
    m_lightsBuffer.alloc(*m_graphicAPI);
}

void Renderer::beginScene(const Renderer::Camera& cam)
{
    utils::uint32 w, h;
    m_window->getWindowSize(&w, &h);
    math::mat4x4 projectionMatrix = cam.projectionMatrix;
    projectionMatrix[0][0] /= (float)w / (float)h;

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

void Renderer::addPointLight(const Renderer::PointLight& pointLight)
{
    utils::uint32 idx = m_lightsBuffer.content().pointLightCount++;
    m_lightsBuffer.content().pointLights[idx] = pointLight;
}

void Renderer::endScene()
{
    m_lightsBuffer.unmap();
    
}

void Renderer::render()
{
    m_graphicAPI->beginFrame();
    m_graphicAPI->beginRenderPass();

    utils::SharedPtr<gfx::GraphicPipeline> gfxPipeline = makeGraphicPipeline(*m_graphicAPI);
    m_graphicAPI->useGraphicsPipeline(gfxPipeline);

    m_graphicAPI->setVertexBuffer(m_vpMatrix.buffer(), gfxPipeline->getVertexBufferIndex("vpMatrixBuffer"));
    m_graphicAPI->setFragmentBuffer(m_lightsBuffer.buffer(), gfxPipeline->getFragmentBufferIndex("lightsBuffer"));

    for (auto& renderable : m_renderables)
    {
        m_graphicAPI->setVertexBuffer(renderable.modelMatrix, gfxPipeline->getVertexBufferIndex("modelMatrixBuffer"));

        m_graphicAPI->setVertexBuffer(renderable.vertexBuffer, 0);
        m_graphicAPI->drawIndexedVertices(renderable.indexBuffer);
    }

    m_graphicAPI->endRenderPass();
    m_graphicAPI->endFrame();
}

}