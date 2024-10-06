/*
 * ---------------------------------------------------
 * ViewportFrameBuffer.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/03 11:18:40
 * ---------------------------------------------------
 */

#ifndef VIEWPORTFRAMEBUFFER_HPP
#define VIEWPORTFRAMEBUFFER_HPP

#include "Graphics/FrameBuffer.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "Graphics/RenderTarget.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/Window.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/Types.hpp"

namespace GE
{

class ViewportFrameBuffer
{
public:
    ViewportFrameBuffer()                           = default;
    ViewportFrameBuffer(const ViewportFrameBuffer&) = default;
    ViewportFrameBuffer(ViewportFrameBuffer&&)      = default;

    void resize(utils::uint32 width, utils::uint32 height);
    void update(gfx::Window&, gfx::GraphicAPI&);

    utils::uint32 width() const { return m_width; }
    utils::uint32 height() const { return m_height; }

    utils::SharedPtr<gfx::Texture> colorTexture() { return m_frameBuffer->colorTexture(); }

    inline utils::SharedPtr<gfx::RenderTarget> getAsRenderTarget() const { return m_frameBuffer.staticCast<gfx::RenderTarget>(); }

    ~ViewportFrameBuffer() = default;

private:
    utils::SharedPtr<gfx::FrameBuffer> m_frameBuffer;

    utils::uint32 m_width = 800;
    utils::uint32 m_height = 600;
    bool m_isDirty = true;

public:
    ViewportFrameBuffer& operator = (const ViewportFrameBuffer&) = default;
    ViewportFrameBuffer& operator = (ViewportFrameBuffer&&)      = default;
};

}

#endif // VIEWPORTFRAMEBUFFER_HPP