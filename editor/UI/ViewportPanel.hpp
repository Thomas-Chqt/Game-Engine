/*
 * ---------------------------------------------------
 * ViewportPanel.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/01 16:20:17
 * ---------------------------------------------------
 */

#ifndef VIEWPORTPANEL_HPP
#define VIEWPORTPANEL_HPP

#include "Graphics/Texture.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/Types.hpp"
#include <utility>

namespace GE
{

class ViewportPanel
{
public:
    ViewportPanel()                     = delete;
    ViewportPanel(const ViewportPanel&) = delete;
    ViewportPanel(ViewportPanel&&)      = delete;
    
    ViewportPanel(const gfx::Texture&);

    inline ViewportPanel& onResize(const utils::Func<void(utils::uint32, utils::uint32)>& f) { return m_onResize = f, *this; }
    inline ViewportPanel& onResize(utils::Func<void(utils::uint32, utils::uint32)>&& f) { return m_onResize = std::move(f), *this; }

    void render();

    ~ViewportPanel() = default;

private:
    const gfx::Texture& m_texture;
    utils::Func<void(utils::uint32, utils::uint32)> m_onResize;

public:
    ViewportPanel& operator = (const ViewportPanel&) = delete;
    ViewportPanel& operator = (ViewportPanel&&)      = delete;
};

}

#endif // VIEWPORTPANEL_HPP