/*
 * ---------------------------------------------------
 * Renderer.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/29 12:14:17
 * ---------------------------------------------------
 */

#include "Renderer/Renderer.hpp"
#include "UtilsCPP/SharedPtr.hpp"

namespace GE
{

Renderer::Renderer(utils::SharedPtr<gfx::GraphicAPI> graphicAPI)
    : m_graphicAPI(std::move(graphicAPI))
{
}

void Renderer::beginScene(const math::vec3f& cameraPos, const math::mat4x4& vpMatrix)
{
    m_cameraPos = cameraPos;
    m_vpMatrix = vpMatrix;
}

void Renderer::endScene()
{
}

}