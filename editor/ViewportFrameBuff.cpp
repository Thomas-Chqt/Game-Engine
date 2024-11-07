/*
 * ---------------------------------------------------
 * ViewportFrameBuff.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/07 12:51:51
 * ---------------------------------------------------
 */

#include "ViewportFrameBuff.hpp"

namespace GE
{

ViewportFrameBuff::ViewportFrameBuff(const gfx::Window& win, const gfx::GraphicAPI& api)
    : m_window(win), m_graphicAPI(api)
{
}

void ViewportFrameBuff::resize(utils::uint32 w, utils::uint32 h)
{
    m_width = w;
    m_height = h;
}

void ViewportFrameBuff::onUpdate()
{
    float xScale, yScale;
    m_window.getFrameBufferScaleFactor(&xScale, &yScale);

    utils::uint32 newFrameBufferWidth = (utils::uint32)((float)m_width * xScale);
    utils::uint32 newFrameBufferHeight = (utils::uint32)((float)m_height * yScale);

    if (m_framebuffer && (m_framebuffer->width() == newFrameBufferWidth && m_framebuffer->height() == newFrameBufferHeight))
        return;
    
    gfx::Texture::Descriptor colorTextureDescriptor;
    colorTextureDescriptor.width = newFrameBufferWidth;
    colorTextureDescriptor.height = newFrameBufferHeight;
    colorTextureDescriptor.pixelFormat = gfx::PixelFormat::BGRA;
    colorTextureDescriptor.usage = gfx::Texture::Usage::ShaderReadAndRenderTarget;
    
    gfx::Texture::Descriptor depthTextureDescriptor = gfx::Texture::Descriptor::depthTextureDescriptor(newFrameBufferWidth, newFrameBufferHeight);

    gfx::FrameBuffer::Descriptor fBuffDesc;
    fBuffDesc.colorTexture = m_graphicAPI.newTexture(colorTextureDescriptor);
    fBuffDesc.depthTexture = m_graphicAPI.newTexture(depthTextureDescriptor);
    m_framebuffer = m_graphicAPI.newFrameBuffer(fBuffDesc);
}

}