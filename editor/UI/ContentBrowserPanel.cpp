/*
 * ---------------------------------------------------
 * ContentBrowserPanel.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "UI/ContentBrowserPanel.hpp"

#include <imgui.h>

#include <string>

constexpr float ELEMENT_SIZE = 60.0f;

namespace GE_Editor
{

void ContentBrowserPanel::renderElement(const std::string& name, const void* payload)
{
    std::string childId = name + "childId";
    if (ImGui::BeginChild(childId.c_str(), ImVec2(ELEMENT_SIZE, ELEMENT_SIZE + ImGui::GetFrameHeightWithSpacing())))
    {
        ImGui::Button(name.c_str(), ImVec2(ELEMENT_SIZE, ELEMENT_SIZE));
        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload(m_payloadType, payload, m_payloadSize);
            ImGui::Text("%s", name.c_str());
            ImGui::EndDragDropSource();
        }
        ImGui::Text("%s", name.c_str());
    }
    ImGui::EndChild();

    m_lineWith += ELEMENT_SIZE + 7.5F;
    if (m_lineWith < ImGui::GetContentRegionAvail().x - ELEMENT_SIZE)
        ImGui::SameLine();
    else
        m_lineWith = 0.0F;
}

}
