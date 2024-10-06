/*
 * ---------------------------------------------------
 * ViewportPanel.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/12 19:34:38
 * ---------------------------------------------------
 */

#ifndef VIEWPORTPANEL_HPP
#define VIEWPORTPANEL_HPP

#include "UtilsCPP/Func.hpp"
#include "Scene.hpp"
#include "UtilsCPP/Types.hpp"
#include "ViewportFrameBuffer.hpp"

namespace GE
{

class ViewportPanel
{
public:
    ViewportPanel()                     = delete;
    ViewportPanel(const ViewportPanel&) = delete;
    ViewportPanel(ViewportPanel&&)      = delete;

    ViewportPanel(ViewportFrameBuffer&);

    inline ViewportPanel& onResize(const utils::Func<void(utils::uint32, utils::uint32)>& f) { return m_onResize = f, *this; }
    inline ViewportPanel& setOnSceneDrop(const utils::Func<void(Scene*)>& f) { return m_onSceneDrop = f, *this; }
    void render();

    ~ViewportPanel() = default;

private:
    ViewportFrameBuffer& m_viewportFBuff; 

    utils::Func<void(utils::uint32, utils::uint32)> m_onResize;
    utils::Func<void(Scene*)> m_onSceneDrop;

public:
    ViewportPanel& operator = (const ViewportPanel&) = delete;
    ViewportPanel& operator = (ViewportPanel&&)      = delete;
};

}

#endif // VIEWPORTPANEL_HPP