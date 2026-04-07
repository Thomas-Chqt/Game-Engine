/*
 * ---------------------------------------------------
 * ViewportPanel.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/01 16:21:18
 * ---------------------------------------------------
 */

#include "UI/ViewportPanel.hpp"

#include <imgui.h>

#include <cstdint>
#include <string>
#include <utility>
#include <variant>

namespace GE_Editor
{

ViewportPanel::ViewportPanel(std::pair<uint32_t, uint32_t> size)
    : m_size(size)
{
}

void ViewportPanel::render()
{
    static std::variant<std::string, uint64_t> textureIdPlaceholder = std::string("viewportBackBuffer");
    if (ImGui::Begin("viewport"))
    {
        ImVec2 contentRegionAvai = ImGui::GetContentRegionAvail();
        uint32_t newWidth = contentRegionAvai.x <= 0 ? 1 : contentRegionAvai.x;
        uint32_t newHeight = contentRegionAvai.y <= 0 ? 1 : contentRegionAvai.y;

        ImGui::Image(&textureIdPlaceholder, contentRegionAvai);

        if (newWidth != m_size.first || newHeight != m_size.second)
        {
            if (m_onResize)
                m_onResize({newWidth, newHeight});
        }
    }
    ImGui::End();
}

}
