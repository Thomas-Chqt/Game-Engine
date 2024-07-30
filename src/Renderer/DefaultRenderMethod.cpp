/*
 * ---------------------------------------------------
 * DefaultRenderMethod.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/30 13:11:55
 * ---------------------------------------------------
 */

#include "Renderer/DefaultRenderMethod.hpp"
#include "Graphics/GraphicPipeline.hpp"
#include "Renderer/RenderMethod.hpp"

namespace GE
{

void DefaultRenderMethod::setGraphicAPI(const utils::SharedPtr<gfx::GraphicAPI>& api)
{
    RenderMethod::setGraphicAPI(api);
    m_vpMatrixBufferIdx = m_graphicPipeline->getVertexBufferIndex("vpMatrixBuffer");
    m_modelMatrixBufferIdx = m_graphicPipeline->getVertexBufferIndex("modelMatrixBuffer");
    m_lightsBufferIdx = m_graphicPipeline->getFragmentBufferIndex("lightsBuffer");
}

gfx::Shader::Descriptor DefaultRenderMethod::vertexShaderDescriptor()
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

gfx::Shader::Descriptor DefaultRenderMethod::fragmentShaderDescriptor()
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

gfx::GraphicPipeline::Descriptor DefaultRenderMethod::graphicPipelineDescriptor()
{
    gfx::VertexLayout vertexLayout;
    vertexLayout.attributes.append({gfx::VertexAttributeFormat::vec3f, offsetof(DefaultRenderMethod::Vertex, pos)});
    vertexLayout.attributes.append({gfx::VertexAttributeFormat::vec2f, offsetof(DefaultRenderMethod::Vertex, uv)});
    vertexLayout.attributes.append({gfx::VertexAttributeFormat::vec3f, offsetof(DefaultRenderMethod::Vertex, normal)});
    vertexLayout.attributes.append({gfx::VertexAttributeFormat::vec3f, offsetof(DefaultRenderMethod::Vertex, tangent)});
    vertexLayout.stride = sizeof(DefaultRenderMethod::Vertex);

    gfx::GraphicPipeline::Descriptor gfxPipelineDesc;
    gfxPipelineDesc.vertexLayout = vertexLayout;

    return gfxPipelineDesc;
}

}