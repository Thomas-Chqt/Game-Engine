/*
 * ---------------------------------------------------
 * RenderMethod.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/30 12:37:48
 * ---------------------------------------------------
 */

#include "Game-Engine/RenderMethod.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "Graphics/GraphicPipeline.hpp"
#include "UtilsCPP/SharedPtr.hpp"

namespace GE
{

void RenderMethod::build(gfx::GraphicAPI& api)
{
    m_graphicAPI = &api;

    utils::SharedPtr<gfx::Shader> vs = m_graphicAPI->newShader(vertexShaderDescriptor());
    utils::SharedPtr<gfx::Shader> fs = m_graphicAPI->newShader(fragmentShaderDescriptor());
    gfx::GraphicPipeline::Descriptor gfxPipelineDesc = graphicPipelineDescriptor();
    gfxPipelineDesc.vertexShader = vs;
    gfxPipelineDesc.fragmentShader = fs;
    m_graphicPipeline = m_graphicAPI->newGraphicsPipeline(gfxPipelineDesc);
}

void RenderMethod::use()
{
    m_graphicAPI->useGraphicsPipeline(m_graphicPipeline);
}

void RenderMethod::setVPMatrixBuffer(const utils::SharedPtr<gfx::Buffer>& buffer)
{
    m_graphicAPI->setVertexBuffer(buffer, vpMatrixBufferIdx());
}

void RenderMethod::setLightsBuffer(const utils::SharedPtr<gfx::Buffer>& buffer)
{
    m_graphicAPI->setFragmentBuffer(buffer, lightsBufferIdx());
}

void RenderMethod::setModelMatrixBuffer(const utils::SharedPtr<gfx::Buffer>& buffer)
{
    m_graphicAPI->setVertexBuffer(buffer, modelMatrixBufferIdx());
}

}