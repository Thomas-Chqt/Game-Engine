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
#include <imgui.h>

namespace GE
{

void ViewportPanel::render(const utils::SharedPtr<gfx::FrameBuffer>& fb)
{
    if (ImGui::Begin("viewport"))
    {
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        viewportSize.x = viewportSize.x == 0 ? 1 : viewportSize.x;
        viewportSize.y = viewportSize.y == 0 ? 1 : viewportSize.y;
        m_contentRegionAvail = math::vec2f{viewportSize.x, viewportSize.y};

        utils::SharedPtr<gfx::Texture> colorTexture = fb->colorTexture();
        ImGui::Image(colorTexture->imguiTextureId(), viewportSize, colorTexture->imguiUV0(), colorTexture->imguiUV1());
        
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