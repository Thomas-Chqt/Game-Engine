/*
 * ---------------------------------------------------
 * Renderer.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/29 12:14:17
 * ---------------------------------------------------
 */

#include "Renderer/Renderer.hpp"
#include "Math/Vector.hpp"
#include "Renderer/RenderMethod.hpp"
#include "UtilsCPP/SharedPtr.hpp"

namespace GE
{

void Renderer::setGraphicAPI(const utils::SharedPtr<gfx::GraphicAPI>& api)
{
    m_graphicAPI = api;
    m_defaultRenderMethod.setGraphicAPI(m_graphicAPI);
    m_vpMatrixBuffer.alloc(*m_graphicAPI);
    m_lightsBuffer.alloc(*m_graphicAPI);
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

void Renderer::addPointLight(const math::vec3f& pos, const math::rgb& color, float intentsity)
{
    auto& newLight = m_lightsBuffer.content().pointLights[m_lightsBuffer.content().pointLightCount++];
    newLight.pos = pos;
    newLight.color = color;
    newLight.intentsity = intentsity;
}

void Renderer::endScene()
{
    m_lightsBuffer.unmap();

    RenderMethod::ShaderGlobalDatas shaderGlobalDatas;
    shaderGlobalDatas.vpMatrixBuffer = m_vpMatrixBuffer.buffer();
    shaderGlobalDatas.lightsBuffer = m_lightsBuffer.buffer();

    m_graphicAPI->beginFrame();
    m_graphicAPI->beginRenderPass();

    m_defaultRenderMethod.use(shaderGlobalDatas);

    for (auto& renderable : m_renderables)
    {
        m_defaultRenderMethod.setModelMatrixBuffer(renderable.modelMatrix);

        m_graphicAPI->setVertexBuffer(renderable.vertexBuffer, 0);
        m_graphicAPI->drawIndexedVertices(renderable.indexBuffer);
    }

    m_graphicAPI->endRenderPass();
    m_graphicAPI->endFrame();
}

}