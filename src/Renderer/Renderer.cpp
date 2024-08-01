/*
 * ---------------------------------------------------
 * Renderer.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/29 12:14:17
 * ---------------------------------------------------
 */

#include "Renderer/Renderer.hpp"
#include "Renderer/RenderMethod.hpp"
#include "Graphics/Enums.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "Math/Matrix.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class DefaultRenderMethod : public RenderMethod
{
public:
    gfx::Shader::Descriptor vertexShaderDescriptor() override
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

        return shaderDescriptor;
    }

    gfx::Shader::Descriptor fragmentShaderDescriptor() override
    {
        gfx::Shader::Descriptor shaderDescriptor;

        shaderDescriptor.type = gfx::ShaderType::fragment;
        #ifdef GFX_BUILD_METAL
            shaderDescriptor.mtlShaderLibPath = MTL_SHADER_LIB;
            shaderDescriptor.mtlFunction = "default_fs";
        #endif
        #ifdef GFX_BUILD_OPENGL
            shaderDescriptor.openglCode = utils::String::contentOfFile(OPENGL_SHADER_DIR"/default.fs");
        #endif

        return shaderDescriptor;
    }

    gfx::GraphicPipeline::Descriptor graphicPipelineDescriptor() override
    {
        gfx::VertexLayout vertexLayout;
        vertexLayout.attributes.append({gfx::VertexAttributeFormat::vec3f, offsetof(Vertex, pos)});
        vertexLayout.attributes.append({gfx::VertexAttributeFormat::vec2f, offsetof(Vertex, uv)});
        vertexLayout.attributes.append({gfx::VertexAttributeFormat::vec3f, offsetof(Vertex, normal)});
        vertexLayout.stride = sizeof(Vertex);

        gfx::GraphicPipeline::Descriptor gfxPipelineDesc;
        gfxPipelineDesc.vertexLayout = vertexLayout;

        return gfxPipelineDesc;
    }

    inline utils::uint64 vpMatrixBufferIdx() override { return m_graphicPipeline->getVertexBufferIndex("vpMatrixBuffer"); }
    inline utils::uint64 modelMatrixBufferIdx() override { return m_graphicPipeline->getVertexBufferIndex("modelMatrixBuffer"); }
    inline utils::uint64 lightsBufferIdx() override { return m_graphicPipeline->getFragmentBufferIndex("lightsBuffer"); }

    ~DefaultRenderMethod() override = default;
};


Renderer::Renderer(gfx::GraphicAPI& api)
    : m_graphicAPI(api),
      m_vpMatrixBuffer(m_graphicAPI),
      m_lightsBuffer(m_graphicAPI)
{
    m_renderMethods.append(utils::makeUnique<DefaultRenderMethod>().staticCast<RenderMethod>());
    m_defaultRenderMethod = m_renderMethods.first();
    m_defaultRenderMethod->build(m_graphicAPI);
}

void Renderer::beginScene(const math::mat4x4& vpMatrix)
{
    m_renderables.clear();

    m_vpMatrixBuffer.map();
    m_vpMatrixBuffer.content() = vpMatrix;
    m_vpMatrixBuffer.unmap();

    m_lightsBuffer.map();
    m_lightsBuffer.content().pointLightCount = 0;
}

void Renderer::endScene()
{
    m_lightsBuffer.unmap();

    m_graphicAPI.beginFrame();
    if (m_makeUIFunc)
        m_graphicAPI.beginImguiRenderPass();
    else
        m_graphicAPI.beginRenderPass();

    useRenderMethod(m_defaultRenderMethod);

    for (auto& renderable : m_renderables)
    {
        m_usedRenderMethod->setModelMatrixBuffer(renderable.modelMatrix);

        m_graphicAPI.setVertexBuffer(renderable.vertexBuffer, 0);
        m_graphicAPI.drawIndexedVertices(renderable.indexBuffer);
    }

    if (m_makeUIFunc)
        m_makeUIFunc();

    m_graphicAPI.endRenderPass();
    m_graphicAPI.endFrame();
}

void Renderer::useRenderMethod(RenderMethod* method)
{
    m_usedRenderMethod = method;
    m_usedRenderMethod->use();
    m_usedRenderMethod->setVPMatrixBuffer(m_vpMatrixBuffer.buffer());
    m_usedRenderMethod->setLightsBuffer(m_lightsBuffer.buffer());
}

}