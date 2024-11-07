/*
 * ---------------------------------------------------
 * ViewportFrameBuff.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/07 12:48:52
 * ---------------------------------------------------
 */

#ifndef VIEWPORTFRAMEBUFF_HPP
#define VIEWPORTFRAMEBUFF_HPP

#include "Graphics/FrameBuffer.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "Graphics/Window.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/Types.hpp"

namespace GE
{

class ViewportFrameBuff
{
public:
    ViewportFrameBuff()                         = delete;
    ViewportFrameBuff(const ViewportFrameBuff&) = delete;
    ViewportFrameBuff(ViewportFrameBuff&&)      = delete;

    ViewportFrameBuff(const gfx::Window&, const gfx::GraphicAPI&);
    
    void resize(utils::uint32 w, utils::uint32 h);

    void onUpdate();

    ~ViewportFrameBuff() = default;

private:
    const gfx::Window& m_window;
    const gfx::GraphicAPI& m_graphicAPI;

    utils::SharedPtr<gfx::FrameBuffer> m_framebuffer;
    utils::uint32 m_width = 800;
    utils::uint32 m_height = 600;
    
public:
    ViewportFrameBuff& operator = (const ViewportFrameBuff&) = delete;
    ViewportFrameBuff& operator = (ViewportFrameBuff&&)      = delete;

    inline operator const utils::SharedPtr<gfx::FrameBuffer>& () const { return m_framebuffer; }
};

}

#endif // VIEWPORTFRAMEBUFF_HPP