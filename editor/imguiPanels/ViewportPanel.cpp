/*
 * ---------------------------------------------------
 * ViewportPanel.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/12 22:51:06
 * ---------------------------------------------------
 */

#include "imguiPanels/ViewportPanel.hpp"
#include "Scene.hpp"
#include "UtilsCPP/Types.hpp"
#include <imgui.h>

namespace GE
{

ViewportPanel::ViewportPanel(const utils::SharedPtr<gfx::Texture>& image)
    : m_image(image)
{
}

void ViewportPanel::render()
{
    static utils::uint32 s_viewportWidth = 0;
    static utils::uint32 s_viewportHeight = 0;

    if (ImGui::Begin("viewport"))
    {
        ImVec2 contentRegionAvai = ImGui::GetContentRegionAvail();
        contentRegionAvai.x = contentRegionAvai.x == 0 ? 1 : contentRegionAvai.x;
        contentRegionAvai.y = contentRegionAvai.y == 0 ? 1 : contentRegionAvai.y;

        if (contentRegionAvai.x != s_viewportWidth || contentRegionAvai.y != s_viewportHeight)
        {
            s_viewportWidth = contentRegionAvai.x;
            s_viewportHeight = contentRegionAvai.y;
            if (m_onResize)
                m_onResize(s_viewportWidth, s_viewportHeight);
        }

        ImGui::Image(m_image->imguiTextureId(), contentRegionAvai, m_image->imguiUV0(), m_image->imguiUV1());
        
        if (m_onSceneDrop && ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_DND"))
            {
                IM_ASSERT(payload->DataSize == sizeof(Scene*));
                m_onSceneDrop(*(Scene**)payload->Data);
            }
            ImGui::EndDragDropTarget();
        }
    }
    ImGui::End();
}

}