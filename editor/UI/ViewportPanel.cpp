/*
 * ---------------------------------------------------
 * ViewportPanel.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/01 16:21:18
 * ---------------------------------------------------
 */

#include "UI/ViewportPanel.hpp"

namespace GE
{

ViewportPanel::ViewportPanel(const gfx::Texture& texture)
    : m_texture(texture)
{
}

void ViewportPanel::render()
{
    static utils::uint32 width = 800;
    static utils::uint32 height = 600;

    if (ImGui::Begin("viewport"))
    {
        ImVec2 contentRegionAvai = ImGui::GetContentRegionAvail();
        utils::uint32 newWidth = contentRegionAvai.x == 0 ? 1 : contentRegionAvai.x;
        utils::uint32 newHeight = contentRegionAvai.y == 0 ? 1 : contentRegionAvai.y;

        ImGui::Image(m_texture.imguiTextureId(), contentRegionAvai, m_texture.imguiUV0(), m_texture.imguiUV1());

        if (newWidth != height || newWidth != width)
        {
            width = newWidth;
            height = newHeight;
            if (m_onResize)
                m_onResize(width, height);
        }
    }
    ImGui::End();
}

}