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

ViewportPanel::ViewportPanel(ViewportFrameBuffer& viewportFBuff)
    : m_viewportFBuff(viewportFBuff)
{
}

void ViewportPanel::render()
{
    if (ImGui::Begin("viewport"))
    {
        ImVec2 contentRegionAvai = ImGui::GetContentRegionAvail();
        contentRegionAvai.x = contentRegionAvai.x == 0 ? 1 : contentRegionAvai.x;
        contentRegionAvai.y = contentRegionAvai.y == 0 ? 1 : contentRegionAvai.y;

        if (m_onResize && (contentRegionAvai.x != m_viewportFBuff.width() || contentRegionAvai.y != m_viewportFBuff.height()))
            m_onResize((utils::uint32)contentRegionAvai.x, (utils::uint32)contentRegionAvai.y);

        utils::SharedPtr<gfx::Texture> colorTexture = m_viewportFBuff.colorTexture();
        ImGui::Image(colorTexture->imguiTextureId(), contentRegionAvai, colorTexture->imguiUV0(), colorTexture->imguiUV1());
        
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