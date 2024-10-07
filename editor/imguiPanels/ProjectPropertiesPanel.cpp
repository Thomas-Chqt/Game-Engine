/*
 * ---------------------------------------------------
 * ProjectPropertiesPanel.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/07 10:20:34
 * ---------------------------------------------------
 */

#include "ProjectPropertiesPanel.hpp"

namespace GE
{

ProjectPropertiesPanel::ProjectPropertiesPanel(Project& proj)
    : m_project(proj)
{
}

void ProjectPropertiesPanel::render()
{
    bool showProjectProperties = true;
    ImGui::OpenPopup("Project properties");
    if (ImGui::BeginPopupModal("Project properties", &showProjectProperties))
    {
        projectNameEdit();
        ImGui::EndPopup();
    }
    if (showProjectProperties == false && m_onClose)
        m_onClose();
        
}

void ProjectPropertiesPanel::projectNameEdit()
{
    char buff[32];
    std::strncpy(buff, m_project.name(), sizeof(buff));
    ImGui::InputText("Name", buff, sizeof(buff));
    m_project.setName(utils::String(buff));
}

}