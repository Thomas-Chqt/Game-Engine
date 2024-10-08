/*
 * ---------------------------------------------------
 * ProjectPropertiesPopupModal.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/07 10:20:34
 * ---------------------------------------------------
 */

#include "ProjectPropertiesPopupModal.hpp"

namespace GE
{

ProjectPropertiesPopupModal::ProjectPropertiesPopupModal(bool& isPresented, Project& proj)
    : m_isPresented(isPresented), m_project(proj)
{
}

void ProjectPropertiesPopupModal::render()
{
    if (m_isPresented)
        ImGui::OpenPopup("Project properties");
    if (ImGui::BeginPopupModal("Project properties", &m_isPresented, ImGuiWindowFlags_AlwaysAutoResize))
    {
        projectNameEdit();
        resourceDirEdit();

        ImGui::EndPopup();
    }
    if (m_isPresented == false && m_onClose)
        m_onClose();
}

void ProjectPropertiesPopupModal::projectNameEdit()
{
    char buff[32];
    std::strncpy(buff, m_project.name, sizeof(buff));
    ImGui::InputText("Name", buff, sizeof(buff));
    m_project.name = utils::String(buff);
}

void ProjectPropertiesPopupModal::resourceDirEdit()
{
    char buff[32];
    std::strncpy(buff, m_project.ressourcesDir, sizeof(buff));
    ImGui::InputText("Ressource directory", buff, sizeof(buff));
    m_project.ressourcesDir = utils::String(buff);
}

}