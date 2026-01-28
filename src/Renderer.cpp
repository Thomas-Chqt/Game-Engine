/*
 * ---------------------------------------------------
 * Renderer.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/09 15:54:42
 * ---------------------------------------------------
 */

#include "Game-Engine/Renderer.hpp"

#include <cassert>

namespace GE
{

Renderer::Renderer(gfx::Device* device) : m_device(device)
{
    for (auto& frameData : m_frameDatas)
    {
        frameData.commandBufferPool = m_device->newCommandBufferPool();
        assert(frameData.commandBufferPool);

        frameData.parameterBlockPool = m_device->newParameterBlockPool({
            .maxUniformBuffers = 2, .maxTextures = 0, .maxSamplers = 0
        });
        assert(frameData.parameterBlockPool);
    }
}

}
