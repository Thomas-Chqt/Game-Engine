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

#include "UtilsCPP/SharedPtr.hpp"
#include "Graphics/FrameBuffer.hpp"
#include "Math/Vector.hpp"
#include "UtilsCPP/Func.hpp"
#include "Scene.hpp"

namespace GE
{

class ViewportPanel
{
public:
    ViewportPanel()                     = default;
    ViewportPanel(const ViewportPanel&) = delete;
    ViewportPanel(ViewportPanel&&)      = delete;

    void render(const utils::SharedPtr<gfx::FrameBuffer>&);

    inline math::vec2f contentRegionAvail() { return m_contentRegionAvail; }

    inline void setOnSceneDrop(const utils::Func<void(Scene*)>& f) { m_onSceneDrop = f; }

    ~ViewportPanel() = default;

private:
    math::vec2f m_contentRegionAvail = {800, 600};

    utils::Func<void(Scene*)> m_onSceneDrop;

public:
    ViewportPanel& operator = (const ViewportPanel&) = delete;
    ViewportPanel& operator = (ViewportPanel&&)      = delete;
};

}

#endif // VIEWPORTPANEL_HPP