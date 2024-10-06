/*
 * ---------------------------------------------------
 * ViewportFrameBuffer.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/03 11:27:07
 * ---------------------------------------------------
 */

#include "ViewportFrameBuffer.hpp"

namespace GE
{

void ViewportFrameBuffer::resize(utils::uint32 width, utils::uint32 height)
{
    m_width = width;
    m_height = height;
}

void ViewportFrameBuffer::update(gfx::Window& window, gfx::GraphicAPI& api)
{
    if (m_isDirty == false)
        return;

    float xScale, yScale;
    window.getFrameBufferScaleFactor(&xScale, &yScale);

    utils::uint32 newFrameBufferWidth = (utils::uint32)((float)m_width * xScale);
    utils::uint32 newFrameBufferHeight = (utils::uint32)((float)m_height * yScale);
    
    gfx::Texture::Descriptor colorTextureDescriptor;
    colorTextureDescriptor.width = newFrameBufferWidth;
    colorTextureDescriptor.height = newFrameBufferHeight;
    colorTextureDescriptor.pixelFormat = gfx::PixelFormat::BGRA;
    colorTextureDescriptor.usage = gfx::Texture::Usage::ShaderReadAndRenderTarget;
    
    gfx::Texture::Descriptor depthTextureDescriptor = gfx::Texture::Descriptor::depthTextureDescriptor(newFrameBufferWidth, newFrameBufferHeight);

    gfx::FrameBuffer::Descriptor fBuffDesc;
    fBuffDesc.colorTexture = api.newTexture(colorTextureDescriptor);
    fBuffDesc.depthTexture = api.newTexture(depthTextureDescriptor);
    m_frameBuffer = api.newFrameBuffer(fBuffDesc);
}

}