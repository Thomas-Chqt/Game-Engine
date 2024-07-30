/*
 * ---------------------------------------------------
 * RenderMethod.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/30 12:37:48
 * ---------------------------------------------------
 */

#include "Renderer/RenderMethod.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "Graphics/GraphicPipeline.hpp"
#include "UtilsCPP/SharedPtr.hpp"

namespace GE
{

void RenderMethod::setGraphicAPI(const utils::SharedPtr<gfx::GraphicAPI>& api)
{
    m_graphicAPI = api;

    utils::SharedPtr<gfx::Shader> vs = m_graphicAPI->newShader(vertexShaderDescriptor());
    utils::SharedPtr<gfx::Shader> fs = m_graphicAPI->newShader(fragmentShaderDescriptor());
    gfx::GraphicPipeline::Descriptor gfxPipelineDesc = graphicPipelineDescriptor();
    gfxPipelineDesc.vertexShader = vs;
    gfxPipelineDesc.fragmentShader = fs;
    m_graphicPipeline = m_graphicAPI->newGraphicsPipeline(gfxPipelineDesc);
}

void RenderMethod::use(const ShaderGlobalDatas& shData)
{
    m_graphicAPI->useGraphicsPipeline(m_graphicPipeline);
    m_graphicAPI->setVertexBuffer(shData.vpMatrixBuffer, vpMatrixBufferIdx());
    m_graphicAPI->setFragmentBuffer(shData.lightsBuffer, lightsBufferIdx());
}

void RenderMethod::setModelMatrixBuffer(const utils::SharedPtr<gfx::Buffer>& buffer)
{
    m_graphicAPI->setVertexBuffer(buffer, modelMatrixBufferIdx());
}

}